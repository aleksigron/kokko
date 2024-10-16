#include "Rendering/Renderer.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>

#include "Core/Core.hpp"
#include "Core/Sort.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/Engine.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/EnvironmentSystem.hpp"
#include "Graphics/GraphicsFeature.hpp"
#include "Graphics/GraphicsFeatureBloom.hpp"
#include "Graphics/GraphicsFeatureCommandList.hpp"
#include "Graphics/GraphicsFeatureDeferredLighting.hpp"
#include "Graphics/GraphicsFeatureSkybox.hpp"
#include "Graphics/GraphicsFeatureSsao.hpp"
#include "Graphics/GraphicsFeatureTonemapping.hpp"
#include "Graphics/Scene.hpp"

#include "Math/AABB.hpp"
#include "Math/Rectangle.hpp"
#include "Math/Intersect3D.hpp"

#include "Memory/Allocator.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/MeshComponentSystem.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/CommandEncoder.hpp"
#include "Rendering/RenderDebugSettings.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderPassType.hpp"
#include "Rendering/RenderTargetContainer.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Rendering/CommandBuffer.hpp"
#include "Rendering/RenderPassDescriptor.hpp"
#include "Rendering/RenderPass.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ResourceManagers.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

namespace kokko
{

namespace
{
RenderPassType ConvertTransparencyToPass(TransparencyType transparency)
{
	switch (transparency)
	{
	case TransparencyType::Opaque:
	case TransparencyType::AlphaTest:
		return RenderPassType::OpaqueGeometry;
	case TransparencyType::Skybox:
		return RenderPassType::Skybox;
	case TransparencyType::TransparentMix:
	case TransparencyType::TransparentAdd:
	case TransparencyType::TransparentSub:
		return RenderPassType::Transparent;
	default:
		return RenderPassType::OpaqueGeometry;
	}
}
}

struct DebugNormalUniformBlock
{
	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;
	alignas(16) Vec4f baseColor;
	alignas(4) float normalLength;
};

Renderer::Renderer(
	Allocator* allocator,
	kokko::render::Device* renderDevice,
	kokko::render::CommandEncoder* commandEncoder,
	kokko::MeshComponentSystem* componentSystem,
	Scene* scene,
	CameraSystem* cameraSystem,
	LightManager* lightManager,
	kokko::EnvironmentSystem* environmentSystem,
	const kokko::ResourceManagers& resourceManagers,
	const RenderDebugSettings* renderDebug) :
	allocator(allocator),
	device(renderDevice),
	encoder(commandEncoder),
	componentSystem(componentSystem),
	targetFramebufferId(0),
	viewportData(nullptr),
	viewportCount(0),
	viewportIndexFullscreen(0),
	uniformStagingBuffer(allocator),
	objectUniformBufferLists{ Array<render::BufferId>(allocator) },
	currentFrameIndex(0),
	scene(scene),
	cameraSystem(cameraSystem),
	lightManager(lightManager),
	environmentSystem(environmentSystem),
	shaderManager(resourceManagers.shaderManager),
	modelManager(resourceManagers.modelManager),
	materialManager(resourceManagers.materialManager),
	textureManager(resourceManagers.textureManager),
	renderDebug(renderDebug),
	lockCullingCamera(false),
	commandList(allocator),
	objectVisibility(allocator),
	lightResultArray(allocator),
	graphicsFeatures(allocator),
	normalDebugBufferId(0)
{
	KOKKO_PROFILE_FUNCTION();

	renderGraphResources = MakeUnique<kokko::RenderGraphResources>(allocator, renderDevice);
	renderTargetContainer = MakeUnique<RenderTargetContainer>(allocator, allocator, renderDevice);
	postProcessRenderer = MakeUnique<kokko::PostProcessRenderer>(
		allocator, encoder, modelManager, shaderManager, renderTargetContainer.Get());

	shadowMaterial = MaterialId::Null;
	fallbackMeshMaterial = MaterialId::Null;

	objectUniformBlockStride = 0;
	objectsPerUniformBuffer = 0;
}

Renderer::~Renderer()
{
}

void Renderer::Initialize()
{
	KOKKO_PROFILE_FUNCTION();
	{
		auto scope = device->CreateDebugScope(0, kokko::ConstStringView("Renderer_InitResources"));

		int aligment = 0;
		device->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &aligment);

		objectUniformBlockStride = (sizeof(TransformUniformBlock) + aligment - 1) / aligment * aligment;
		objectsPerUniformBuffer = ObjectUniformBufferSize / objectUniformBlockStride;

		postProcessRenderer->Initialize();

		{
			KOKKO_PROFILE_SCOPE("Allocate viewport data");

			// Allocate viewport data storage
			void* buf = allocator->Allocate(sizeof(RenderViewport) * MaxViewportCount, "Renderer::viewportData");
			viewportData = static_cast<RenderViewport*>(buf);

			// Create uniform buffer objects
			render::BufferId buffers[MaxViewportCount];
			device->CreateBuffers(MaxViewportCount, buffers);

			for (size_t i = 0; i < MaxViewportCount; ++i)
			{
				device->SetBufferStorage(buffers[i], sizeof(ViewportUniformBlock), nullptr, BufferStorageFlags::Dynamic);

				kokko::ConstStringView label("Renderer viewport uniform buffer");
				device->SetObjectLabel(RenderObjectType::Buffer, buffers[i].i, label);

				viewportData[i].uniformBlockObject = buffers[i];
			}
		}

		{
			const char* path = "engine/materials/forward/shadow_depth.material";
			shadowMaterial = materialManager->FindMaterialByPath(kokko::ConstStringView(path));
		}

		{
			const char* path = "engine/materials/deferred_geometry/fallback.material";
			fallbackMeshMaterial = materialManager->FindMaterialByPath(kokko::ConstStringView(path));
		}
	}

	{
		auto scope = device->CreateDebugScope(0, kokko::ConstStringView("Renderer_InitFeatures"));

		// Deferred lighting pass

		auto ssaoFeature = allocator->MakeNew<kokko::GraphicsFeatureSsao>(allocator);
		ssaoFeature->SetOrder(0);

		auto lightingFeature = allocator->MakeNew<kokko::GraphicsFeatureDeferredLighting>(allocator);
		lightingFeature->SetOrder(1);

		// Post process pass

		auto bloomFeature = allocator->MakeNew<kokko::GraphicsFeatureBloom>(allocator);
		bloomFeature->SetOrder(0);

		auto tonemappingFeature = allocator->MakeNew<kokko::GraphicsFeatureTonemapping>();
		tonemappingFeature->SetOrder(1);

		graphicsFeatures.Reserve(5);
		graphicsFeatures.PushBack(ssaoFeature);
		graphicsFeatures.PushBack(lightingFeature);
		graphicsFeatures.PushBack(allocator->MakeNew<kokko::GraphicsFeatureSkybox>());
		graphicsFeatures.PushBack(bloomFeature);
		graphicsFeatures.PushBack(tonemappingFeature);

		kokko::GraphicsFeature::InitializeParameters parameters;
		parameters.renderDevice = device;
		parameters.modelManager = modelManager;
		parameters.shaderManager = shaderManager;

		for (auto feature : graphicsFeatures)
		{
			feature->Initialize(parameters);
		}
	}
}

void Renderer::Deinitialize()
{
	kokko::GraphicsFeature::InitializeParameters parameters;
	parameters.renderDevice = device;
	parameters.modelManager = modelManager;
	parameters.shaderManager = shaderManager;

	for (auto feature : graphicsFeatures)
	{
		feature->Deinitialize(parameters);
		allocator->MakeDelete(feature);
	}

	graphicsFeatures.Clear();

	for (unsigned int i = 0; i < FramesInFlightCount; ++i)
	{
		if (objectUniformBufferLists[i].GetCount() > 0)
		{
			Array<render::BufferId>& list = objectUniformBufferLists[i];
			device->DestroyBuffers(static_cast<unsigned int>(list.GetCount()), list.GetData());
			list.Clear();
		}
	}

	if (viewportData != nullptr)
	{
		for (size_t i = 0; i < MaxViewportCount; ++i)
		{
			if (viewportData[i].uniformBlockObject != 0)
			{
				device->DestroyBuffers(1, &(viewportData[i].uniformBlockObject));
				viewportData[i].uniformBlockObject = render::BufferId();
			}
		}

		allocator->Deallocate(viewportData);
		viewportData = nullptr;
		viewportCount = 0;
	}
}

void Renderer::Render(Window* window, const Optional<CameraParameters>& editorCamera, const render::Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();


#ifdef KOKKO_USE_METAL
	kokko::NativeSurface* surface = window->GetNativeSurface();
	kokko::TextureHandle texture = window->GetNativeSurfaceTexture();
	kokko::CommandBuffer* buffer = device->CreateCommandBuffer(allocator);
	kokko::RenderPassDescriptor passDescriptor;
	passDescriptor.colorAttachments.PushBack(kokko::RenderPassColorAttachment{
		texture,
		kokko::AttachmentLoadAction::Clear,
		kokko::AttachmentStoreAction::Store,
		Vec4f(0.0f, 0.6f, 0.7f, 1.0f)
		});
	kokko::RenderPass* renderPass = buffer->CreateRenderPass(passDescriptor, allocator);

	allocator->MakeDelete(renderPass);

	buffer->Present(surface);
	buffer->Commit();
	window->ReleaseNativeSurface();

	allocator->MakeDelete(buffer);

	return;
#endif

	if (targetFramebuffer.IsInitialized() == false)
		return;

	if (targetFramebuffer.GetSize() != renderGraphResources->GetFullscreenViewportSize())
		renderTargetContainer->DestroyAllRenderTargets();

	renderGraphResources->VerifyResourcesAreCreated(targetFramebuffer.GetSize());

	targetFramebufferId = targetFramebuffer.GetFramebufferId();

	unsigned int objectDrawCount = PopulateCommandList(editorCamera, targetFramebuffer);
	UpdateUniformBuffers(objectDrawCount);

	intptr_t objectDrawsProcessed = 0;

	uint64_t lastVpIdx = MaxViewportCount;
	render::ShaderId lastShaderProgram = render::ShaderId();
	MaterialId lastMaterialId = MaterialId{ 0 };

	Array<render::BufferId>& objUniformBuffers = objectUniformBufferLists[currentFrameIndex];

	CameraParameters cameraParams = GetCameraParameters(editorCamera, targetFramebuffer);

	kokko::GraphicsFeature::RenderParameters featureRenderParams
	{
		postProcessRenderer.Get(),
		modelManager,
		shaderManager,
		textureManager,
		cameraSystem,
		environmentSystem,
		lightManager,
		*renderDebug,
		cameraParams,
		ArrayView(&viewportData[0], viewportCount),
		viewportIndexFullscreen,
		viewportIndicesShadowCascade.start,
		viewportIndicesShadowCascade.GetLength(),
		renderGraphResources.Get(),
		targetFramebuffer.GetFramebufferId(),
		encoder,
		0,
		0
	};

	auto scope = encoder->CreateDebugScope(0, kokko::ConstStringView("Renderer_Render"));

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;

		// If command is not control command, draw object
		if (ParseControlCommand(command) == false)
		{
			uint64_t mat = renderOrder.materialId.GetValue(command);
			uint64_t vpIdx = renderOrder.viewportIndex.GetValue(command);
			const RenderViewport& viewport = viewportData[vpIdx];

			if (mat != RenderOrderConfiguration::CallbackMaterialId)
			{
				MaterialId matId = MaterialId{ static_cast<uint16_t>(mat) };

				if (matId == MaterialId::Null)
					matId = fallbackMeshMaterial;

				uint64_t objIdx = renderOrder.renderObject.GetValue(command);
				uint16_t meshPart = static_cast<uint16_t>(renderOrder.meshPart.GetValue(command));

				// Update viewport uniform block
				if (vpIdx != lastVpIdx)
				{
					render::BufferId ubo = viewportData[vpIdx].uniformBlockObject;
					encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, ubo);

					lastVpIdx = vpIdx;
				}

				if (matId != lastMaterialId)
				{
					lastMaterialId = matId;
					render::ShaderId matShaderId = materialManager->GetMaterialShaderDeviceId(matId);
					render::BufferId matUniformBuffer = materialManager->GetMaterialUniformBufferId(matId);

					if (matShaderId != lastShaderProgram)
					{
						encoder->UseShaderProgram(matShaderId);
						lastShaderProgram = matShaderId;
					}

					BindMaterialTextures(materialManager->GetMaterialUniforms(matId));

					// Bind material uniform block to shader
					encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Material, matUniformBuffer);
				}

				// Bind object transform uniform block to shader

				intptr_t bufferIndex = objectDrawsProcessed / objectsPerUniformBuffer;
				intptr_t objectInBuffer = objectDrawsProcessed % objectsPerUniformBuffer;
				size_t rangeSize = static_cast<size_t>(objectUniformBlockStride);

				encoder->BindBufferRange(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object,
					objUniformBuffers[bufferIndex], objectInBuffer * objectUniformBlockStride, rangeSize);

				MeshId meshId = componentSystem->data.mesh[objIdx];
				auto& mesh = modelManager->GetModelMeshes(meshId.modelId)[meshId.meshIndex];
				auto& part = modelManager->GetModelMeshParts(meshId.modelId)[mesh.partOffset + meshPart];
				encoder->BindVertexArray(part.vertexArrayId);
				encoder->DrawIndexed(mesh.primitiveMode, mesh.indexType, part.count, part.indexOffset, 0);

				objectDrawsProcessed += 1;
			}
			else // Render with callback
			{
				uint64_t featureIndex = renderOrder.featureIndex.GetValue(command);

				featureRenderParams.renderingViewportIndex = vpIdx;
				featureRenderParams.featureObjectId = renderOrder.featureObjectId.GetValue(command);

				graphicsFeatures[featureIndex]->Render(featureRenderParams);

				// Reset state cache
				lastVpIdx = MaxViewportCount;
				lastShaderProgram = render::ShaderId();
				lastMaterialId = MaterialId{ 0 };

				// TODO: manage sampler state more robustly
				encoder->BindSampler(0, render::SamplerId());

				// TODO: Figure how to restore viewport and other relevant state
			}
		}
	}

	commandList.Clear();

	renderTargetContainer->ConfirmAllTargetsAreUnused();

	currentFrameIndex = (currentFrameIndex + 1) % FramesInFlightCount;

	targetFramebufferId = render::FramebufferId();
}

void Renderer::BindMaterialTextures(const kokko::UniformData& materialUniforms) const
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int usedTextures = 0;

	for (auto& uniform : materialUniforms.GetTextureUniforms())
	{
		switch (uniform.type)
		{
		case kokko::UniformDataType::Tex2D:
		case kokko::UniformDataType::TexCube:
			encoder->BindTextureToShader(uniform.uniformLocation, usedTextures, uniform.textureObject);
			++usedTextures;
			break;

		default:
			break;
		}
	}
}

void Renderer::SetLockCullingCamera(bool lockEnable)
{
	lockCullingCamera = lockEnable;
}

const Mat4x4f& Renderer::GetCullingCameraTransform() const
{
	return lockCullingCameraTransform.forward;
}

void Renderer::UpdateUniformBuffers(size_t objectDrawCount)
{
	KOKKO_PROFILE_FUNCTION();

	auto scope = device->CreateDebugScope(0, kokko::ConstStringView("Renderer_UpdateBuffers"));

	size_t buffersRequired = (objectDrawCount + objectsPerUniformBuffer - 1) / objectsPerUniformBuffer;

	Array<render::BufferId>& objUniformBuffers = objectUniformBufferLists[currentFrameIndex];

	// Create new object transform uniform buffers if needed
	if (buffersRequired > objUniformBuffers.GetCount())
	{
		size_t currentCount = objUniformBuffers.GetCount();
		objUniformBuffers.Resize(buffersRequired);

		unsigned int addCount = static_cast<unsigned int>(buffersRequired - currentCount);
		device->CreateBuffers(addCount, objUniformBuffers.GetData() + currentCount);

		for (size_t i = currentCount; i < buffersRequired; ++i)
		{
			device->SetBufferStorage(objUniformBuffers[i], ObjectUniformBufferSize, nullptr, BufferStorageFlags::Dynamic);

			kokko::ConstStringView label("Renderer object uniform buffer");
			device->SetObjectLabel(RenderObjectType::Buffer, objUniformBuffers[i].i, label);
		}
	}

	intptr_t prevBufferIndex = -1;

	uniformStagingBuffer.Resize(ObjectUniformBufferSize);
	unsigned char* stagingBuffer = uniformStagingBuffer.GetData();

	size_t objectDrawsProcessed = 0;

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;
		uint64_t mat = renderOrder.materialId.GetValue(command);
		uint64_t vpIdx = renderOrder.viewportIndex.GetValue(command);
		uint64_t objIdx = renderOrder.renderObject.GetValue(command);

		// Is regular draw command
		if (IsDrawCommand(command) && mat != RenderOrderConfiguration::CallbackMaterialId)
		{
			intptr_t bufferIndex = objectDrawsProcessed / objectsPerUniformBuffer;
			intptr_t objectInBuffer = objectDrawsProcessed % objectsPerUniformBuffer;

			if (bufferIndex != prevBufferIndex)
			{
				if (prevBufferIndex >= 0)
				{
					device->SetBufferSubData(objUniformBuffers[prevBufferIndex], 0, ObjectUniformBufferSize, stagingBuffer);
				}

				prevBufferIndex = bufferIndex;
			}

			TransformUniformBlock* tu = reinterpret_cast<TransformUniformBlock*>(stagingBuffer + objectUniformBlockStride * objectInBuffer);

			const Mat4x4f& model = componentSystem->data.transform[objIdx];
			tu->MVP = viewportData[vpIdx].viewProjection * model;
			tu->MV = viewportData[vpIdx].view.inverse * model;
			tu->M = model;

			objectDrawsProcessed += 1;
		}
	}

	if (prevBufferIndex >= 0)
	{
		unsigned int updateSize = static_cast<unsigned int>((objectDrawsProcessed % objectsPerUniformBuffer) * objectUniformBlockStride);
		device->SetBufferSubData(objUniformBuffers[prevBufferIndex], 0, updateSize, stagingBuffer);
	}
}

bool Renderer::IsDrawCommand(uint64_t orderKey)
{
	return renderOrder.command.GetValue(orderKey) == static_cast<uint64_t>(RendererCommandType::Draw);
}

bool Renderer::ParseControlCommand(uint64_t orderKey)
{
	if (renderOrder.command.GetValue(orderKey) == static_cast<uint64_t>(RendererCommandType::Draw))
		return false;

	uint64_t commandTypeInt = renderOrder.commandType.GetValue(orderKey);
	RendererControlType control = static_cast<RendererControlType>(commandTypeInt);

	switch (control)
	{
	case RendererControlType::BeginViewport:
	{
		uint32_t viewportIndex = static_cast<uint32_t>(renderOrder.viewportIndex.GetValue(orderKey));

		if (viewportIndex == 0)
		{
			encoder->DepthTestEnable();
			encoder->SetDepthTestFunction(RenderDepthCompareFunc::Greater);
			encoder->DepthWriteEnable();
			encoder->SetCullFace(RenderCullFace::Back);
			encoder->ScissorTestDisable();
			encoder->BlendingDisable();
			encoder->SetClearDepth(0.0f);
		}

		if (viewportIndex == viewportIndicesShadowCascade.start)
		{
			const auto& shadowBuffer = renderGraphResources->GetShadowBuffer();

			// Bind shadow framebuffer before any shadow cascade draws
			encoder->BindFramebuffer(shadowBuffer.GetFramebufferId());

			// Set viewport size to full framebuffer size before clearing
			encoder->SetViewport(0, 0, shadowBuffer.GetWidth(), shadowBuffer.GetHeight());

			// Clear shadow framebuffer
			encoder->Clear(ClearMask{false, true, false});
		}

		if (viewportIndex >= viewportIndicesShadowCascade.start &&
			viewportIndex < viewportIndicesShadowCascade.end)
		{
			const auto& rect = viewportData[viewportIndex].viewportRectangle;

			encoder->SetViewport(rect.position.x, rect.position.y, rect.size.x, rect.size.y);
		}

		if (viewportIndex == viewportIndexFullscreen)
		{
			const auto& rect = viewportData[viewportIndexFullscreen].viewportRectangle;

			encoder->SetDepthTestFunction(RenderDepthCompareFunc::Greater);
			encoder->SetClearColor(Vec4f{ 0.0f, 0.0f, 0.0f, 0.0f });
			encoder->SetViewport(rect.position.x, rect.position.y, rect.size.x, rect.size.y);
			encoder->BindFramebuffer(renderGraphResources->GetGeometryBuffer().GetFramebufferId());
			encoder->Clear(ClearMask{ true, true, false });
		}

		break;
	}

	case RendererControlType::BeginPass:
	{
		uint32_t viewportIndex = static_cast<uint32_t>(renderOrder.viewportIndex.GetValue(orderKey));
		auto pass = static_cast<RenderPassType>(renderOrder.viewportPass.GetValue(orderKey));

		if (viewportIndex == viewportIndexFullscreen)
		{
			if (pass == RenderPassType::Skybox)
			{
				const auto& rect = viewportData[viewportIndexFullscreen].viewportRectangle;

				encoder->DepthTestEnable();
				encoder->SetDepthTestFunction(RenderDepthCompareFunc::Equal);
				encoder->DepthWriteDisable();
				encoder->SetViewport(rect.position.x, rect.position.y, rect.size.x, rect.size.y);
			}
			else if (pass == RenderPassType::Transparent)
			{
				encoder->SetDepthTestFunction(RenderDepthCompareFunc::Greater);
				encoder->BlendingEnable();

				// Set mix blending
				encoder->BlendFunction(RenderBlendFactor::SrcAlpha, RenderBlendFactor::OneMinusSrcAlpha);
			}
		}
		break;
	}
	}

	return true;
}

float CalculateDepth(const Vec3f& objPos, const Vec3f& eyePos, const Vec3f& eyeForward, float farMinusNear, float minusNear)
{
	return (Vec3f::Dot(objPos - eyePos, eyeForward) - minusNear) / farMinusNear;
}

CameraParameters Renderer::GetCameraParameters(const Optional<CameraParameters>& editorCamera, const render::Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	if (editorCamera.HasValue())
	{
		return editorCamera.GetValue();
	}
	else
	{
		Entity cameraEntity = cameraSystem->GetActiveCamera();

		CameraParameters result;

		if (cameraEntity != Entity::Null) {
			SceneObjectId cameraSceneObj = scene->Lookup(cameraEntity);
			result.transform.forward = scene->GetWorldTransform(cameraSceneObj);

			Optional<Mat4x4f> viewOpt = result.transform.forward.GetInverse();
			if (viewOpt.HasValue())
				result.transform.inverse = viewOpt.GetValue();
			else
				KK_LOG_ERROR("Renderer: camera's transform couldn't be inverted");

			CameraId cameraId = cameraSystem->Lookup(cameraEntity);
			result.projection = cameraSystem->GetProjection(cameraId);
			result.projection.SetAspectRatio(targetFramebuffer.GetWidth(), targetFramebuffer.GetHeight());
		}

		return result;
	}
}

unsigned int Renderer::PopulateCommandList(const Optional<CameraParameters>& editorCamera, const render::Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	const float mainViewportMinObjectSize = 50.0f;
	const float shadowViewportMinObjectSize = 30.0f;

	// Get camera transforms

	CameraParameters cameraParameters = GetCameraParameters(editorCamera, targetFramebuffer);

	Mat4x4fBijection cameraTransforms = cameraParameters.transform;
	ProjectionParameters projectionParams = cameraParameters.projection;
	Mat4x4f cameraProjection = projectionParams.GetProjectionMatrix(true);

	Mat4x4fBijection cullingTransform;

	if (lockCullingCamera)
		cullingTransform = lockCullingCameraTransform;
	else
	{
		cullingTransform = cameraTransforms;
		lockCullingCameraTransform = cameraTransforms;
	}

	int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
	unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
	Vec2i shadowCascadeSize(shadowSide, shadowSide);
	Vec2i shadowTextureSize(shadowSide * shadowCascadeCount, shadowSide);
	Mat4x4fBijection cascadeViewTransforms[CascadedShadowMap::MaxCascadeCount];
	ProjectionParameters lightProjections[CascadedShadowMap::MaxCascadeCount];

	// Reset the used viewport count
	viewportCount = 0;

	// Update directional light viewports
	lightManager->GetDirectionalLights(lightResultArray);

	ViewportUniformBlock viewportUniforms;

	{
		auto scope = device->CreateDebugScope(0, kokko::ConstStringView("Renderer_UpdateViewports"));
		for (size_t i = 0, count = lightResultArray.GetCount(); i < count; ++i)
		{
			LightId id = lightResultArray[i];

			if (lightManager->GetShadowCasting(id) &&
				viewportCount + shadowCascadeCount + 1 <= MaxViewportCount)
			{
				Mat3x3f orientation = lightManager->GetOrientation(id);
				Vec3f lightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);

				CascadedShadowMap::CalculateCascadeFrusta(lightDir, cameraTransforms.forward, projectionParams, cascadeViewTransforms, lightProjections);

				viewportIndicesShadowCascade = Range<unsigned int>(viewportCount, viewportCount + shadowCascadeCount);

				for (unsigned int cascade = 0; cascade < shadowCascadeCount; ++cascade)
				{
					unsigned int vpIdx = viewportCount;
					viewportCount += 1;

					bool reverseDepth = true;

					const Mat4x4f& forwardTransform = cascadeViewTransforms[cascade].forward;
					const Mat4x4f& inverseTransform = cascadeViewTransforms[cascade].inverse;

					RenderViewport& vp = viewportData[vpIdx];
					vp.position = (forwardTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
					vp.forward = (forwardTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
					vp.farMinusNear = lightProjections[cascade].orthographicFar - lightProjections[cascade].orthographicNear;
					vp.minusNear = -lightProjections[cascade].orthographicNear;
					vp.objectMinScreenSizePx = shadowViewportMinObjectSize;
					vp.view.forward = forwardTransform;
					vp.view.inverse = inverseTransform;
					vp.projection = lightProjections[cascade].GetProjectionMatrix(reverseDepth);
					vp.viewProjection = vp.projection * vp.view.inverse;
					vp.viewportRectangle.size = shadowCascadeSize;
					vp.viewportRectangle.position = Vec2i(cascade * shadowSide, 0);
					vp.frustum.Update(lightProjections[cascade], forwardTransform);

					viewportUniforms.VP = vp.viewProjection;
					viewportUniforms.V = vp.view.inverse;
					viewportUniforms.P = vp.projection;

					device->SetBufferSubData(vp.uniformBlockObject, 0, sizeof(ViewportUniformBlock), &viewportUniforms);
				}
			}
		}

		lightResultArray.Clear();

		{
			// Add the fullscreen viewport
			unsigned int vpIdx = viewportCount;
			viewportCount += 1;

			bool reverseDepth = true;

			Rectanglei viewportRect{ 0, 0, targetFramebuffer.GetWidth(), targetFramebuffer.GetHeight() };

			RenderViewport& vp = viewportData[vpIdx];
			vp.position = (cameraTransforms.forward * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
			vp.forward = (cameraTransforms.forward * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
			vp.farMinusNear = projectionParams.perspectiveFar - projectionParams.perspectiveNear;
			vp.minusNear = -projectionParams.perspectiveNear;
			vp.objectMinScreenSizePx = mainViewportMinObjectSize;
			vp.view.forward = cameraTransforms.forward;
			vp.view.inverse = cameraTransforms.inverse;
			vp.projection = cameraProjection;
			vp.viewProjection = vp.projection * vp.view.inverse;
			vp.viewportRectangle = viewportRect;
			vp.frustum.Update(projectionParams, cullingTransform.forward);

			viewportUniforms.VP = vp.viewProjection;
			viewportUniforms.V = vp.view.inverse;
			viewportUniforms.P = vp.projection;

			device->SetBufferSubData(vp.uniformBlockObject, 0, sizeof(ViewportUniformBlock), &viewportUniforms);

			this->viewportIndexFullscreen = vpIdx;
		}
	}

	unsigned int numShadowViewports = viewportIndicesShadowCascade.GetLength();

	// Create control commands for beginning of viewports and render passes

	constexpr uint32_t firstViewportIndex = 0;
	commandList.AddControl(0, RenderPassType::OpaqueGeometry, RendererControlType::BeginViewport, firstViewportIndex);

	for (unsigned int vpIdx = viewportIndicesShadowCascade.start; vpIdx < viewportIndicesShadowCascade.end; ++vpIdx)
		if (vpIdx != firstViewportIndex)
			commandList.AddControl(vpIdx, RenderPassType::OpaqueGeometry, RendererControlType::BeginViewport);

	unsigned int fsvp = viewportIndexFullscreen;

	if (fsvp != firstViewportIndex)
		commandList.AddControl(fsvp, RenderPassType::OpaqueGeometry, RendererControlType::BeginViewport);

	commandList.AddControl(fsvp, RenderPassType::Skybox, RendererControlType::BeginPass);
	commandList.AddControl(fsvp, RenderPassType::Transparent, RendererControlType::BeginPass);

	// Create draw commands for render objects in scene

	unsigned int componentCount = componentSystem->data.count;
	unsigned int visRequired = BitPack::CalculateRequired(componentCount);
	objectVisibility.Resize(static_cast<size_t>(visRequired) * viewportCount);

	const uint8_t compareTrIdx = static_cast<uint8_t>(TransparencyType::AlphaTest);

	BitPack* vis[MaxViewportCount];

	for (size_t vpIdx = 0; vpIdx < viewportCount; ++vpIdx)
	{
		vis[vpIdx] = objectVisibility.GetData() + visRequired * vpIdx;

		const FrustumPlanes& frustum = viewportData[vpIdx].frustum;
		const Mat4x4f& viewProjection = viewportData[vpIdx].viewProjection;
		const Vec2i viewPortSize = viewportData[vpIdx].viewportRectangle.size;
		float minSize = viewportData[vpIdx].objectMinScreenSizePx / (viewPortSize.x * viewPortSize.y);

		Intersect::FrustumAABBMinSize(frustum, viewProjection, minSize, componentCount,
			componentSystem->data.bounds, vis[vpIdx]);
	}

	unsigned int objectDrawCount = 0;

	for (unsigned int i = 1; i < componentCount; ++i)
	{
		Vec3f objPos = (componentSystem->data.transform[i] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

		// Test visibility in shadow viewports
		for (unsigned int vpIdx = 0, count = numShadowViewports; vpIdx < count; ++vpIdx)
		{
			if (BitPack::Get(vis[vpIdx], i))
			{
				const RenderViewport& vp = viewportData[vpIdx];

				float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);
				auto transparencies = componentSystem->GetTransparencyTypes(MeshComponentId{ i });
				uint8_t partCount = static_cast<uint8_t>(transparencies.GetCount());
				for (size_t partIndex = 0; partIndex != partCount; ++partIndex)
				{
					if (static_cast<uint8_t>(transparencies[partIndex]) <= compareTrIdx)
					{
						// TODO: Render all mesh parts in one draw call since all of them use the same material
						commandList.AddDraw(vpIdx, RenderPassType::OpaqueGeometry, depth, shadowMaterial, i, partIndex);
					}
				}

				objectDrawCount += 1;
			}
		}

		// Test visibility in fullscreen viewport
		if (BitPack::Get(vis[fsvp], i))
		{
			uint32_t meshIndex = componentSystem->data.mesh[i].meshIndex;
			auto id = MeshComponentId{ i };
			auto materials = componentSystem->GetMaterialIds(id);
			auto transparencies = componentSystem->GetTransparencyTypes(id);
			const RenderViewport& vp = viewportData[fsvp];

			float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

			uint8_t partCount = static_cast<uint8_t>(materials.GetCount());
			for (size_t partIndex = 0; partIndex != partCount; ++partIndex)
			{
				RenderPassType pass = ConvertTransparencyToPass(transparencies[partIndex]);
				commandList.AddDraw(fsvp, pass, depth, materials[partIndex], i, partIndex);

				objectDrawCount += 1;
			}
		}
	}

	{
		auto scope = device->CreateDebugScope(0, kokko::ConstStringView("Renderer_FeatureUpload"));

		kokko::GraphicsFeature::UploadParameters uploadParameters
		{
			postProcessRenderer.Get(),
			modelManager,
			shaderManager,
			textureManager,
			cameraSystem,
			environmentSystem,
			lightManager,
			*renderDebug,
			cameraParameters,
			ArrayView(&viewportData[0], viewportCount),
			viewportIndexFullscreen,
			viewportIndicesShadowCascade.start,
			viewportIndicesShadowCascade.GetLength(),
			renderGraphResources.Get(),
			targetFramebufferId,
			device
		};

		for (auto feature : graphicsFeatures)
		{
			feature->Upload(uploadParameters);
		}
	}

	{
		GraphicsFeature::SubmitParameters submitParams
		{
			postProcessRenderer.Get(),
			modelManager,
			shaderManager,
			textureManager,
			cameraSystem,
			environmentSystem,
			lightManager,
			*renderDebug,
			cameraParameters,
			ArrayView(&viewportData[0], viewportCount),
			viewportIndexFullscreen,
			viewportIndicesShadowCascade.start,
			viewportIndicesShadowCascade.GetLength(),
			renderGraphResources.Get(),
			targetFramebufferId,
			nullptr
		};

		for (size_t i = 0, count = graphicsFeatures.GetCount(); i < count; ++i)
		{
			uint64_t featureIndex = static_cast<uint64_t>(i);
			GraphicsFeatureCommandList featureCommandList(commandList, featureIndex);
			submitParams.commandList = &featureCommandList;
			graphicsFeatures[i]->Submit(submitParams);
		}
	}

	commandList.Sort();

	return objectDrawCount;
}

void Renderer::DebugRender(DebugVectorRenderer* vectorRenderer)
{
	KOKKO_PROFILE_FUNCTION();

	Entity debugEntity = renderDebug->GetDebugEntity();
	if (debugEntity != Entity::Null &&
		renderDebug->IsFeatureEnabled(RenderDebugFeatureFlag::DrawNormals))
	{
		if (normalDebugBufferId == 0)
		{
			device->CreateBuffers(1, &normalDebugBufferId);
			device->SetBufferStorage(
				normalDebugBufferId, sizeof(DebugNormalUniformBlock), nullptr, BufferStorageFlags::Dynamic);
		}

		kokko::MeshComponentId component = componentSystem->Lookup(debugEntity);
		if (component != kokko::MeshComponentId::Null)
		{
			// Draw normals

			auto shaderPath = kokko::ConstStringView("engine/shaders/debug/debug_normal.glsl");
			ShaderId shaderId = shaderManager->FindShaderByPath(shaderPath);
			if (shaderId == ShaderId::Null)
				return;

			const ShaderData& shader = shaderManager->GetShaderData(shaderId);
			encoder->UseShaderProgram(shader.driverId);

			const Mat4x4f& model = componentSystem->data.transform[component.i];
			DebugNormalUniformBlock uniforms;
			uniforms.MVP = viewportData[viewportIndexFullscreen].viewProjection * model;
			uniforms.MV = viewportData[viewportIndexFullscreen].view.inverse * model;
			uniforms.baseColor = Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
			uniforms.normalLength = 0.03f;

			device->SetBufferSubData(normalDebugBufferId, 0,
				static_cast<unsigned int>(sizeof(DebugNormalUniformBlock)), &uniforms);

			encoder->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, normalDebugBufferId);

			MeshId meshId = componentSystem->GetMeshId(component);
			if (meshId != MeshId::Null)
			{
				auto& mesh = modelManager->GetModelMeshes(meshId.modelId)[meshId.meshIndex];
				auto parts = modelManager->GetModelMeshParts(meshId.modelId);
				for (uint16_t idx = mesh.partOffset, end = mesh.partOffset + mesh.partCount; idx != end; ++idx)
				{
					auto& part = parts[idx];
					int meshVertexCount = part.uniqueVertexCount;
					encoder->BindVertexArray(part.vertexArrayId);

					// Geometry shader will turn points into lines
					encoder->Draw(RenderPrimitiveMode::Points, 0, meshVertexCount);
				}
			}
		}
	}

	if (renderDebug->IsFeatureEnabled(RenderDebugFeatureFlag::DrawBounds))
	{
		Color color(1.0f, 1.0f, 1.0f, 1.0f);

		// Draw bounds
		AABB* bounds = componentSystem->data.bounds;
		for (unsigned int idx = 1, count = componentSystem->data.count; idx < count; ++idx)
		{
			Vec3f pos = bounds[idx].center;
			Vec3f scale = bounds[idx].extents * 2.0f;
			Mat4x4f transform = Mat4x4f::Translate(pos) * Mat4x4f::Scale(scale);
			vectorRenderer->DrawWireCube(transform, color);
		}
	}
}

void Renderer::AddGraphicsFeature(kokko::GraphicsFeature* graphicsFeature)
{
	graphicsFeatures.PushBack(graphicsFeature);
}

void Renderer::RemoveGraphicsFeature(kokko::GraphicsFeature* graphicsFeature)
{
	for (size_t i = 0, count = graphicsFeatures.GetCount(); i < count; ++i)
	{
		if (graphicsFeatures[i] == graphicsFeature)
		{
			graphicsFeatures.Remove(i);
			return;
		}
	}
}

} // namespace kokko
