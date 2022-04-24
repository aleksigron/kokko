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

#include "Math/Rectangle.hpp"
#include "Math/BoundingBox.hpp"
#include "Math/Intersect3D.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/CameraSystem.hpp"
#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/CustomRenderer.hpp"
#include "Rendering/Framebuffer.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/MeshComponentSystem.hpp"
#include "Rendering/PostProcessRenderer.hpp"
#include "Rendering/PostProcessRenderPass.hpp"
#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderCommandType.hpp"
#include "Rendering/RenderDebugSettings.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderGraphResources.hpp"
#include "Rendering/RenderTargetContainer.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/Uniform.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ResourceManagers.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "System/Window.hpp"

struct DebugNormalUniformBlock
{
	alignas(16) Mat4x4f MVP;
	alignas(16) Mat4x4f MV;
	alignas(16) Vec4f baseColor;
	alignas(4) float normalLength;
};

Renderer::Renderer(
	Allocator* allocator,
	RenderDevice* renderDevice,
	kokko::MeshComponentSystem* componentSystem,
	Scene* scene,
	CameraSystem* cameraSystem,
	LightManager* lightManager,
	kokko::EnvironmentSystem* environmentSystem,
	const kokko::ResourceManagers& resourceManagers) :
	allocator(allocator),
	device(renderDevice),
	componentSystem(componentSystem),
	renderGraphResources(nullptr),
	renderTargetContainer(nullptr),
	targetFramebufferId(0),
	viewportData(nullptr),
	viewportCount(0),
	viewportIndexFullscreen(0),
	uniformStagingBuffer(allocator),
	objectUniformBufferLists{ Array<unsigned int>(allocator) },
	currentFrameIndex(0),
	scene(scene),
	cameraSystem(cameraSystem),
	lightManager(lightManager),
	environmentSystem(environmentSystem),
	shaderManager(resourceManagers.shaderManager),
	meshManager(resourceManagers.meshManager),
	materialManager(resourceManagers.materialManager),
	textureManager(resourceManagers.textureManager),
	lockCullingCamera(false),
	commandList(allocator),
	objectVisibility(allocator),
	lightResultArray(allocator),
	customRenderers(allocator),
	graphicsFeatures(allocator),
	normalDebugBufferId(0)
{
	KOKKO_PROFILE_FUNCTION();

	renderGraphResources = allocator->MakeNew<kokko::RenderGraphResources>(renderDevice, meshManager);

	renderTargetContainer = allocator->MakeNew<RenderTargetContainer>(allocator, renderDevice);

	postProcessRenderer = allocator->MakeNew<PostProcessRenderer>(
		renderDevice, meshManager, shaderManager, renderTargetContainer);

	shadowMaterial = MaterialId{ 0 };

	objectUniformBlockStride = 0;
	objectsPerUniformBuffer = 0;
}

Renderer::~Renderer()
{
	allocator->MakeDelete(postProcessRenderer);
	allocator->MakeDelete(renderTargetContainer);
}

void Renderer::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	device->CubemapSeamlessEnable();
	device->SetClipBehavior(RenderClipOriginMode::LowerLeft, RenderClipDepthMode::ZeroToOne);

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
		unsigned int buffers[MaxViewportCount];
		device->CreateBuffers(MaxViewportCount, buffers);

		for (size_t i = 0; i < MaxViewportCount; ++i)
		{
			viewportData[i].uniformBlockObject = buffers[i];
			device->BindBuffer(RenderBufferTarget::UniformBuffer, viewportData[i].uniformBlockObject);

			RenderCommandData::SetBufferStorage storage{};
			storage.target = RenderBufferTarget::UniformBuffer;
			storage.size = sizeof(ViewportUniformBlock);
			storage.data = nullptr;
			storage.dynamicStorage = true;
			device->SetBufferStorage(&storage);

			kokko::ConstStringView label("Renderer viewport uniform buffer");
			device->SetObjectLabel(RenderObjectType::Buffer, buffers[i], label);
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
	parameters.meshManager = meshManager;
	parameters.shaderManager = shaderManager;

	for (auto feature : graphicsFeatures)
	{
		feature->Initialize(parameters);
	}
}

void Renderer::Deinitialize()
{
	kokko::GraphicsFeature::InitializeParameters parameters;
	parameters.renderDevice = device;
	parameters.meshManager = meshManager;
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
			Array<uint32_t>& list = objectUniformBufferLists[i];
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
				viewportData[i].uniformBlockObject = 0;
			}
		}

		allocator->Deallocate(viewportData);
		viewportData = nullptr;
		viewportCount = 0;
	}
}

void Renderer::Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();

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
	uint64_t lastShaderProgram = 0;
	const MeshDrawData* draw = nullptr;
	MeshId lastMeshId = MeshId{ 0 };
	MaterialId lastMaterialId = MaterialId{ 0 };

	Array<unsigned int>& objUniformBuffers = objectUniformBufferLists[currentFrameIndex];
	
	CameraParameters cameraParams = GetCameraParameters(editorCamera, targetFramebuffer);

	kokko::GraphicsFeature::RenderParameters featureRenderParams
	{
		device,
		postProcessRenderer,
		meshManager,
		shaderManager,
		textureManager,
		environmentSystem,
		lightManager,
		cameraParams,
		viewportData[viewportIndexFullscreen],
		ArrayView(viewportData + viewportIndicesShadowCascade.start,
			viewportIndicesShadowCascade.end - viewportIndicesShadowCascade.start),
		renderGraphResources,
		targetFramebuffer.GetFramebufferId(),
		0
	};

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;

		// If command is not control command, draw object
		if (ParseControlCommand(command) == false)
		{
			KOKKO_PROFILE_SCOPE("Draw command");

			uint64_t mat = renderOrder.materialId.GetValue(command);
			uint64_t vpIdx = renderOrder.viewportIndex.GetValue(command);
			const RenderViewport& viewport = viewportData[vpIdx];

			if (mat != RenderOrderConfiguration::CallbackMaterialId)
			{
				MaterialId matId = MaterialId{ static_cast<unsigned int>(mat) };

				if (matId == MaterialId::Null)
					matId = fallbackMeshMaterial;

				uint64_t objIdx = renderOrder.renderObject.GetValue(command);

				// Update viewport uniform block
				if (vpIdx != lastVpIdx)
				{
					unsigned int ubo = viewportData[vpIdx].uniformBlockObject;
					device->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Viewport, ubo);

					lastVpIdx = vpIdx;
				}

				if (matId != lastMaterialId)
				{
					lastMaterialId = matId;
					unsigned int matShaderId = materialManager->GetMaterialShaderDeviceId(matId);
					unsigned int matUniformBuffer = materialManager->GetMaterialUniformBufferId(matId);

					if (matShaderId != lastShaderProgram)
					{
						device->UseShaderProgram(matShaderId);
						lastShaderProgram = matShaderId;
					}

					BindMaterialTextures(materialManager->GetMaterialUniforms(matId));

					// Bind material uniform block to shader
					device->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Material, matUniformBuffer);
				}

				// Bind object transform uniform block to shader

				intptr_t bufferIndex = objectDrawsProcessed / objectsPerUniformBuffer;
				intptr_t objectInBuffer = objectDrawsProcessed % objectsPerUniformBuffer;
				size_t rangeSize = static_cast<size_t>(objectUniformBlockStride);

				RenderCommandData::BindBufferRange bind{
					RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object,
					objUniformBuffers[bufferIndex], objectInBuffer * objectUniformBlockStride, rangeSize
				};

				device->BindBufferRange(&bind);

				MeshId mesh = componentSystem->data.mesh[objIdx];

				if (mesh != lastMeshId)
				{
					lastMeshId = mesh;
					draw = meshManager->GetDrawData(mesh);
					device->BindVertexArray(draw->vertexArrayObject);
				}
				device->DrawIndexed(draw->primitiveMode, draw->count, draw->indexType);

				objectDrawsProcessed += 1;
			}
			else // Render with callback
			{
				uint64_t featureIndex = renderOrder.featureIndex.GetValue(command);

				if (renderOrder.isGraphicsFeature.GetValue(command) != 0)
				{
					featureRenderParams.featureObjectId = renderOrder.featureObjectId.GetValue(command);

					graphicsFeatures[featureIndex]->Render(featureRenderParams);
				}
				else
				{
					if (featureIndex > 0 && featureIndex <= customRenderers.GetCount())
					{
						CustomRenderer* customRenderer = customRenderers[featureIndex - 1];

						if (customRenderer != nullptr)
						{
							CustomRenderer::RenderParams params;
							params.viewport = &viewport;
							params.cameraParams = cameraParams;
							params.callbackId = static_cast<unsigned int>(featureIndex);
							params.command = command;
							params.scene = scene;

							customRenderer->RenderCustom(params);
						}
					}
				}

				// Reset state cache
				lastVpIdx = MaxViewportCount;
				lastShaderProgram = 0;
				draw = nullptr;
				lastMeshId = MeshId{ 0 };
				lastMaterialId = MaterialId{ 0 };

				// TODO: manage sampler state more robustly
				device->BindSampler(0, 0);

				// TODO: Figure how to restore viewport and other relevant state
			}
		}
	}

	commandList.Clear();

	renderTargetContainer->ConfirmAllTargetsAreUnused();

	currentFrameIndex = (currentFrameIndex + 1) % FramesInFlightCount;

	customRenderers.Clear();
	targetFramebufferId = 0;
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
			device->SetActiveTextureUnit(usedTextures);
			device->BindTexture(uniform.textureTarget, uniform.textureObject);
			device->SetUniformInt(uniform.uniformLocation, usedTextures);
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

	size_t buffersRequired = (objectDrawCount + objectsPerUniformBuffer - 1) / objectsPerUniformBuffer;

	Array<unsigned int>& objUniformBuffers = objectUniformBufferLists[currentFrameIndex];

	// Create new object transform uniform buffers if needed
	if (buffersRequired > objUniformBuffers.GetCount())
	{
		size_t currentCount = objUniformBuffers.GetCount();
		objUniformBuffers.Resize(buffersRequired);

		unsigned int addCount = static_cast<unsigned int>(buffersRequired - currentCount);
		device->CreateBuffers(addCount, objUniformBuffers.GetData() + currentCount);

		for (size_t i = currentCount; i < buffersRequired; ++i)
		{
			device->BindBuffer(RenderBufferTarget::UniformBuffer, objUniformBuffers[i]);

			RenderCommandData::SetBufferStorage setStorage{};
			setStorage.target = RenderBufferTarget::UniformBuffer;
			setStorage.size = ObjectUniformBufferSize;
			setStorage.dynamicStorage = true;

			device->SetBufferStorage(&setStorage);

			kokko::ConstStringView label("Renderer object uniform buffer");
			device->SetObjectLabel(RenderObjectType::Buffer, objUniformBuffers[i], label);
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
					KOKKO_PROFILE_SCOPE("Update buffer data");

					device->BindBuffer(RenderBufferTarget::UniformBuffer, objUniformBuffers[prevBufferIndex]);
					device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, ObjectUniformBufferSize, stagingBuffer);
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
		KOKKO_PROFILE_SCOPE("Update buffer data");

		unsigned int updateSize = static_cast<unsigned int>((objectDrawsProcessed % objectsPerUniformBuffer) * objectUniformBlockStride);
		device->BindBuffer(RenderBufferTarget::UniformBuffer, objUniformBuffers[prevBufferIndex]);
		device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, updateSize, stagingBuffer);
	}
}

bool Renderer::IsDrawCommand(uint64_t orderKey)
{
	return renderOrder.command.GetValue(orderKey) == static_cast<uint64_t>(RenderCommandType::Draw);
}

bool Renderer::ParseControlCommand(uint64_t orderKey)
{
	KOKKO_PROFILE_FUNCTION();

	if (renderOrder.command.GetValue(orderKey) == static_cast<uint64_t>(RenderCommandType::Draw))
		return false;

	uint64_t commandTypeInt = renderOrder.commandType.GetValue(orderKey);
	RenderControlType control = static_cast<RenderControlType>(commandTypeInt);

	switch (control)
	{
		case RenderControlType::BlendingEnable:
			device->BlendingEnable();
			break;

		case RenderControlType::BlendingDisable:
			device->BlendingDisable();
			break;

		case RenderControlType::BlendFunction:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* blendFn = reinterpret_cast<RenderCommandData::BlendFunctionData*>(data);
			device->BlendFunction(blendFn);

		}
			break;

		case RenderControlType::Viewport:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* viewport = reinterpret_cast<RenderCommandData::ViewportData*>(data);
			device->Viewport(viewport);
		}
			break;

		case RenderControlType::ScissorTestEnable:
			device->ScissorTestEnable();
			break;

		case RenderControlType::ScissorTestDisable:
			device->ScissorTestDisable();
			break;

		case RenderControlType::DepthRange:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* depthRange = reinterpret_cast<RenderCommandData::DepthRangeData*>(data);
			device->DepthRange(depthRange);
		}
			break;

		case RenderControlType::DepthTestEnable:
			device->DepthTestEnable();
			break;

		case RenderControlType::DepthTestDisable:
			device->DepthTestDisable();
			break;

		case RenderControlType::DepthTestFunction:
		{
			uint64_t fn = renderOrder.commandData.GetValue(orderKey);
			device->DepthTestFunction(static_cast<RenderDepthCompareFunc>(fn));
		}
			break;

		case RenderControlType::DepthWriteEnable:
			device->DepthWriteEnable();
			break;

		case RenderControlType::DepthWriteDisable:
			device->DepthWriteDisable();
			break;

		case RenderControlType::CullFaceEnable:
			device->CullFaceEnable();
			break;

		case RenderControlType::CullFaceDisable:
			device->CullFaceDisable();
			break;

		case RenderControlType::CullFaceFront:
			device->CullFaceFront();
			break;

		case RenderControlType::CullFaceBack:
			device->CullFaceBack();
			break;

		case RenderControlType::Clear:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* clearMask = reinterpret_cast<RenderCommandData::ClearMask*>(data);
			device->Clear(clearMask);
		}
			break;

		case RenderControlType::ClearColor:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* color = reinterpret_cast<RenderCommandData::ClearColorData*>(data);
			device->ClearColor(color);
		}
			break;

		case RenderControlType::ClearDepth:
		{
			uint64_t intDepth = renderOrder.commandData.GetValue(orderKey);
			float depth = *reinterpret_cast<float*>(&intDepth);
			device->ClearDepth(depth);
		}
			break;

		case RenderControlType::BindFramebuffer:
		{
			uint64_t offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* bind = reinterpret_cast<RenderCommandData::BindFramebufferData*>(data);
			device->BindFramebuffer(bind);
		}
			break;

		case RenderControlType::FramebufferSrgbEnable:
			device->FramebufferSrgbEnable();
			break;

		case RenderControlType::FramebufferSrgbDisable:
			device->FramebufferSrgbDisable();
			break;
	}

	return true;
}

float CalculateDepth(const Vec3f& objPos, const Vec3f& eyePos, const Vec3f& eyeForward, float farMinusNear, float minusNear)
{
	return (Vec3f::Dot(objPos - eyePos, eyeForward) - minusNear) / farMinusNear;
}

CameraParameters Renderer::GetCameraParameters(const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	if (editorCamera.HasValue())
	{
		return editorCamera.GetValue();
	}
	else
	{
		Entity cameraEntity = scene->GetActiveCameraEntity();

		CameraParameters result;

		SceneObjectId cameraSceneObj = scene->Lookup(cameraEntity);
		result.transform.forward = scene->GetWorldTransform(cameraSceneObj);

		Optional<Mat4x4f> viewOpt = result.transform.forward.GetInverse();
		if (viewOpt.HasValue())
			result.transform.inverse = viewOpt.GetValue();
		else
			KK_LOG_ERROR("Renderer: camera's transform couldn't be inverted");

		CameraId cameraId = cameraSystem->Lookup(cameraEntity);
		result.projection = cameraSystem->GetData(cameraId);
		result.projection.SetAspectRatio(targetFramebuffer.GetWidth(), targetFramebuffer.GetHeight());

		return result;
	}
}

unsigned int Renderer::PopulateCommandList(const Optional<CameraParameters>& editorCamera, const Framebuffer& targetFramebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	const float mainViewportMinObjectSize = 50.0f;
	const float shadowViewportMinObjectSize = 30.0f;

	// Get camera transforms

	CameraParameters cameraParameters = GetCameraParameters(editorCamera, targetFramebuffer);

	{
		kokko::GraphicsFeature::UploadParameters featureParameters
		{
			device,
			cameraParameters,
			viewportData[viewportIndexFullscreen]
		};

		for (auto feature : graphicsFeatures)
		{
			feature->Upload(featureParameters);
		}
	}

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

				device->BindBuffer(RenderBufferTarget::UniformBuffer, vp.uniformBlockObject);
				device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(ViewportUniformBlock), &viewportUniforms);
			}
		}
	}

	lightResultArray.Clear();

	unsigned int numShadowViewports = viewportCount;

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

		device->BindBuffer(RenderBufferTarget::UniformBuffer, vp.uniformBlockObject);
		device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(ViewportUniformBlock), &viewportUniforms);

		this->viewportIndexFullscreen = vpIdx;
	}

	const FrustumPlanes& fullscreenFrustum = viewportData[viewportIndexFullscreen].frustum;

	RenderPass g_pass = RenderPass::OpaqueGeometry;
	RenderPass l_pass = RenderPass::OpaqueLighting;
	RenderPass s_pass = RenderPass::Skybox;
	RenderPass t_pass = RenderPass::Transparent;
	RenderPass p_pass = RenderPass::PostProcess;

	using ctrl = RenderControlType;

	// Before light shadow viewport

	// Set depth test on
	commandList.AddControl(0, g_pass, 0, ctrl::DepthTestEnable);

	// Set depth test function
	commandList.AddControl(0, g_pass, 1, ctrl::DepthTestFunction, static_cast<unsigned int>(RenderDepthCompareFunc::Greater));

	// Enable depth writing
	commandList.AddControl(0, g_pass, 2, ctrl::DepthWriteEnable);

	// Set face culling on
	commandList.AddControl(0, g_pass, 3, ctrl::CullFaceEnable);

	// Set face culling to cull back faces
	commandList.AddControl(0, g_pass, 4, ctrl::CullFaceBack);

	commandList.AddControl(0, g_pass, 5, ctrl::ScissorTestDisable);

	{
		// Set clear depth
		float depth = 0.0f;
		unsigned int* intDepthPtr = reinterpret_cast<unsigned int*>(&depth);

		commandList.AddControl(0, g_pass, 6, ctrl::ClearDepth, *intDepthPtr);
	}

	// Disable blending
	commandList.AddControl(0, g_pass, 7, ctrl::BlendingDisable);

	if (numShadowViewports > 0)
	{
		{
			// Bind shadow framebuffer before any shadow cascade draws
			RenderCommandData::BindFramebufferData data;
			data.target = RenderFramebufferTarget::Framebuffer;
			data.framebuffer = renderGraphResources->GetShadowBuffer().GetFramebufferId();

			commandList.AddControl(0, g_pass, 8, ctrl::BindFramebuffer, sizeof(data), &data);
		}

		{
			// Set viewport size to full framebuffer size before clearing
			RenderCommandData::ViewportData data;
			data.x = 0;
			data.y = 0;
			data.w = renderGraphResources->GetShadowBuffer().GetWidth();
			data.h = renderGraphResources->GetShadowBuffer().GetHeight();

			commandList.AddControl(0, g_pass, 9, ctrl::Viewport, sizeof(data), &data);
		}

		// Clear shadow framebuffer RenderFramebufferTarget::Framebuffer
		{
			RenderCommandData::ClearMask clearMask{ false, true, false };
			commandList.AddControl(0, g_pass, 10, ctrl::Clear, sizeof(clearMask), &clearMask);
		}

		// Enable sRGB conversion for framebuffer
		commandList.AddControl(0, g_pass, 11, ctrl::FramebufferSrgbEnable);

		// For each shadow viewport
		for (unsigned int vpIdx = 0; vpIdx < numShadowViewports; ++vpIdx)
		{
			const RenderViewport& viewport = viewportData[vpIdx];

			// Set viewport size
			RenderCommandData::ViewportData data;
			data.x = viewport.viewportRectangle.position.x;
			data.y = viewport.viewportRectangle.position.y;
			data.w = viewport.viewportRectangle.size.x;
			data.h = viewport.viewportRectangle.size.y;

			commandList.AddControl(vpIdx, g_pass, 12, ctrl::Viewport, sizeof(data), &data);
		}
	}

	// Before fullscreen viewport

	// PASS: OPAQUE GEOMETRY

	unsigned int fsvp = viewportIndexFullscreen;

	// Set depth test function for reverse depth buffer
	commandList.AddControl(0, g_pass, 1, ctrl::DepthTestFunction, static_cast<unsigned int>(RenderDepthCompareFunc::Greater));

	{
		// Set clear color
		RenderCommandData::ClearColorData data{ 0.0f, 0.0f, 0.0f, 0.0f };
		commandList.AddControl(fsvp, g_pass, 0, ctrl::ClearColor, sizeof(data), &data);
	}

	{
		const RenderViewport& viewport = viewportData[viewportIndexFullscreen];

		// Set viewport size
		RenderCommandData::ViewportData data;
		data.x = viewport.viewportRectangle.position.x;
		data.y = viewport.viewportRectangle.position.y;
		data.w = viewport.viewportRectangle.size.x;
		data.h = viewport.viewportRectangle.size.y;

		commandList.AddControl(fsvp, g_pass, 1, ctrl::Viewport, sizeof(data), &data);
	}

	{
		// Bind geometry framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = RenderFramebufferTarget::Framebuffer;
		data.framebuffer = renderGraphResources->GetGeometryBuffer().GetFramebufferId();

		commandList.AddControl(fsvp, g_pass, 2, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	// Clear currently bound RenderFramebufferTarget::Framebuffer
	{
		RenderCommandData::ClearMask clearMask{ true, true, false };
		commandList.AddControl(fsvp, g_pass, 3, ctrl::Clear, sizeof(clearMask), &clearMask);
	}

	// PASS: OPAQUE LIGHTING

	// Deferred lighting and SSAO will be added from graphics features

	// PASS: SKYBOX

	commandList.AddControl(fsvp, s_pass, 0, ctrl::DepthTestEnable);
	commandList.AddControl(fsvp, s_pass, 1, ctrl::DepthTestFunction, static_cast<unsigned int>(RenderDepthCompareFunc::Equal));
	commandList.AddControl(fsvp, s_pass, 2, ctrl::DepthWriteDisable);

	{
		const RenderViewport& viewport = viewportData[viewportIndexFullscreen];

		// Set viewport size
		RenderCommandData::ViewportData data;
		data.x = viewport.viewportRectangle.position.x;
		data.y = viewport.viewportRectangle.position.y;
		data.w = viewport.viewportRectangle.size.x;
		data.h = viewport.viewportRectangle.size.y;

		commandList.AddControl(fsvp, s_pass, 3, ctrl::Viewport, sizeof(data), &data);
	}

	// PASS: TRANSPARENT

	// Before transparent objects

	commandList.AddControl(fsvp, t_pass, 0, ctrl::DepthTestFunction, static_cast<unsigned int>(RenderDepthCompareFunc::Greater));
	commandList.AddControl(fsvp, t_pass, 1, ctrl::BlendingEnable);

	{
		// Set mix blending
		RenderCommandData::BlendFunctionData data;
		data.srcFactor = RenderBlendFactor::SrcAlpha;
		data.dstFactor = RenderBlendFactor::OneMinusSrcAlpha;

		commandList.AddControl(fsvp, t_pass, 2, ctrl::BlendFunction, sizeof(data), &data);
	}

	// PASS: POST PROCESS
	// These draw commands will be added from graphics features

	// Create draw commands for render objects in scene

	unsigned int componentCount = componentSystem->data.count;
	unsigned int visRequired = BitPack::CalculateRequired(componentCount);
	objectVisibility.Resize(static_cast<size_t>(visRequired) * viewportCount);

	const unsigned int compareTrIdx = static_cast<unsigned int>(TransparencyType::AlphaTest);

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
			if (BitPack::Get(vis[vpIdx], i) &&
				static_cast<unsigned int>(componentSystem->data.transparency[i]) <= compareTrIdx)
			{
				const RenderViewport& vp = viewportData[vpIdx];

				float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

				commandList.AddDraw(vpIdx, RenderPass::OpaqueGeometry, depth, shadowMaterial, i);

				objectDrawCount += 1;
			}
		}

		// Test visibility in fullscreen viewport
		if (BitPack::Get(vis[fsvp], i))
		{
			MaterialId mat = componentSystem->data.material[i];
			const RenderViewport& vp = viewportData[fsvp];

			float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

			RenderPass pass = static_cast<RenderPass>(componentSystem->data.transparency[i]);
			commandList.AddDraw(fsvp, pass, depth, mat, i);

			objectDrawCount += 1;
		}
	}

	for (size_t i = 0, count = customRenderers.GetCount(); i < count; ++i)
	{
		if (customRenderers[i] != nullptr)
		{
			CustomRenderer::CommandParams params;
			params.fullscreenViewport = this->viewportIndexFullscreen;
			params.commandList = &commandList;
			params.callbackId = static_cast<unsigned int>(i + 1);
			params.scene = scene;

			customRenderers[i]->AddRenderCommands(params);
		}
	}

	for (size_t i = 0, count = graphicsFeatures.GetCount(); i < count; ++i)
	{
		uint64_t featureIndex = static_cast<uint64_t>(i);
		kokko::GraphicsFeatureCommandList commandList(commandList, viewportIndexFullscreen, featureIndex);
		kokko::GraphicsFeature::SubmitParameters params{ commandList };
		graphicsFeatures[i]->Submit(params);
	}

	commandList.Sort();

	return objectDrawCount;
}

unsigned int Renderer::AddCustomRenderer(CustomRenderer* customRenderer)
{
	for (size_t i = 0, count = customRenderers.GetCount(); i < count; ++i)
	{
		if (customRenderers[i] == nullptr)
		{
			customRenderers[i] = customRenderer;

			return static_cast<unsigned int>(i + 1);
		}
	}

	customRenderers.PushBack(customRenderer);
	return static_cast<unsigned int>(customRenderers.GetCount());
}

void Renderer::RemoveCustomRenderer(unsigned int callbackId)
{
	if (callbackId == customRenderers.GetCount())
	{
		customRenderers.PopBack();
	}
	else if (callbackId < customRenderers.GetCount() && callbackId > 0)
	{
		customRenderers[callbackId - 1] = nullptr;
	}
}

void Renderer::DebugRender(DebugVectorRenderer* vectorRenderer, const kokko::RenderDebugSettings& settings)
{
	KOKKO_PROFILE_FUNCTION();

	Entity debugEntity = settings.GetDebugEntity();
	if (debugEntity != Entity::Null &&
		settings.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawNormals))
	{
		if (normalDebugBufferId == 0)
		{
			RenderBufferUsage usage = RenderBufferUsage::DynamicDraw;

			device->CreateBuffers(1, &normalDebugBufferId);

			device->BindBuffer(RenderBufferTarget::UniformBuffer, normalDebugBufferId);

			RenderCommandData::SetBufferStorage storage{};
			storage.target = RenderBufferTarget::UniformBuffer;
			storage.size = sizeof(DebugNormalUniformBlock);
			storage.data = nullptr;
			storage.dynamicStorage = true;
			device->SetBufferStorage(&storage);
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
			device->UseShaderProgram(shader.driverId);

			const Mat4x4f& model = componentSystem->data.transform[component.i];
			DebugNormalUniformBlock uniforms;
			uniforms.MVP = viewportData[viewportIndexFullscreen].viewProjection * model;
			uniforms.MV = viewportData[viewportIndexFullscreen].view.inverse * model;
			uniforms.baseColor = Vec4f(0.0f, 1.0f, 1.0f, 1.0f);
			uniforms.normalLength = 0.03f;

			device->BindBuffer(RenderBufferTarget::UniformBuffer, normalDebugBufferId);
			device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0,
				static_cast<unsigned int>(sizeof(DebugNormalUniformBlock)), &uniforms);

			device->BindBufferBase(RenderBufferTarget::UniformBuffer, UniformBlockBinding::Object, normalDebugBufferId);

			MeshId mesh = componentSystem->GetMeshId(component);
			if (mesh != MeshId::Null)
			{
				int meshVertexCount = meshManager->GetUniqueVertexCount(mesh);
				const MeshDrawData* draw = meshManager->GetDrawData(mesh);
				device->BindVertexArray(draw->vertexArrayObject);

				// Geometry shader will turn points into lines
				device->Draw(RenderPrimitiveMode::Points, 0, meshVertexCount);
			}
		}
	}

	if (settings.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawBounds))
	{
		Color color(1.0f, 1.0f, 1.0f, 1.0f);

		// Draw bounds
		BoundingBox* bounds = componentSystem->data.bounds;
		for (unsigned int idx = 1, count = componentSystem->data.count; idx < count; ++idx)
		{
			Vec3f pos = bounds[idx].center;
			Vec3f scale = bounds[idx].extents * 2.0f;
			Mat4x4f transform = Mat4x4f::Translate(pos) * Mat4x4f::Scale(scale);
			vectorRenderer->DrawWireCube(transform, color);
		}
	}
}
