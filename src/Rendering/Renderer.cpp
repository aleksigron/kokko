#include "Rendering/Renderer.hpp"

#include <cstring>
#include <cstdio>

#include "Core/Sort.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Math/Rectangle.hpp"
#include "Math/BoundingBox.hpp"
#include "Math/Intersect3D.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Camera.hpp"
#include "Rendering/CascadedShadowMap.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderCommandType.hpp"
#include "Rendering/RenderViewport.hpp"
#include "Rendering/StaticUniformBuffer.hpp"
#include "Rendering/ScreenSpaceAmbientOcclusion.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scene/Scene.hpp"

#include "System/IncludeOpenGL.hpp"
#include "System/Window.hpp"

struct RendererFramebuffer
{
	unsigned int framebuffer;

	int width;
	int height;
};

struct TonemapUniformBlock
{
	alignas(16) float exposure;
};

Renderer::Renderer(
	Allocator* allocator,
	RenderDevice* renderDevice,
	LightManager* lightManager,
	ShaderManager* shaderManager,
	MeshManager* meshManager,
	MaterialManager* materialManager) :
	allocator(allocator),
	device(renderDevice),
	ssao(nullptr),
	framebufferData(nullptr),
	framebufferCount(0),
	framebufferTextures(nullptr),
	framebufferTextureCount(0),
	viewportData(nullptr),
	viewportCount(0),
	viewportIndexFullscreen(0),
	objectUniformBuffers(allocator),
	entityMap(allocator),
	lightManager(lightManager),
	shaderManager(shaderManager),
	meshManager(meshManager),
	materialManager(materialManager),
	lockCullingCamera(false),
	commandList(allocator),
	objectVisibility(allocator),
	lightResultArray(allocator),
	customRenderers(allocator)
{
	ssao = allocator->MakeNew<ScreenSpaceAmbientOcclusion>(allocator, renderDevice, shaderManager);

	fullscreenMesh = MeshId{ 0 };
	lightingShaderId = ShaderId{ 0 };
	tonemappingShaderId = ShaderId{ 0 };
	shadowMaterial = MaterialId{ 0 };
	lightingUniformBufferId = 0;

	gBufferAlbedoTextureIndex = 0;
	gBufferNormalTextureIndex = 0;
	gBufferMaterialTextureIndex = 0;
	fullscreenDepthTextureIndex = 0;
	shadowDepthTextureIndex = 0;
	lightAccumulationTextureIndex = 0;

	tonemapUniformBufferId = 0;

	uniformBufferOffsetAlignment = 0;
	objectUniformBlockStride = 0;
	objectsPerUniformBuffer = 0;

	deferredLightingCallback = AddCustomRenderer(this);
	tonemappingCallback = AddCustomRenderer(this);

	data = InstanceData{};
	data.count = 1; // Reserve index 0 as RenderObjectId::Null value

	this->ReallocateRenderObjects(512);
}

Renderer::~Renderer()
{
	this->Deinitialize();

	allocator->Deallocate(data.buffer);
}

void Renderer::Initialize(Window* window)
{
	device->GetIntegerValue(RenderDeviceParameter::UniformBufferOffsetAlignment, &uniformBufferOffsetAlignment);

	objectUniformBlockStride = (sizeof(TransformUniformBlock) * 2 - 1) / uniformBufferOffsetAlignment * uniformBufferOffsetAlignment;
	objectsPerUniformBuffer = ObjectUniformBufferSize / objectUniformBlockStride;

	Vec2i frameBufferSizei = window->GetFrameBufferSize();

	ssao->Initialize(frameBufferSizei);

	{
		// Allocate framebuffer data storage
		void* buf = this->allocator->Allocate(sizeof(RendererFramebuffer) * MaxFramebufferCount);
		framebufferData = static_cast<RendererFramebuffer*>(buf);
	}

	{
		// Allocate framebuffer texture info storage
		void* buf = this->allocator->Allocate(sizeof(unsigned int) * MaxFramebufferTextureCount);
		framebufferTextures = static_cast<unsigned int*>(buf);
	}

	{
		// Allocate viewport data storage
		void* buf = this->allocator->Allocate(sizeof(RenderViewport) * MaxViewportCount);
		viewportData = static_cast<RenderViewport*>(buf);

		// Create uniform buffer objects
		unsigned int buffers[MaxViewportCount];
		device->CreateBuffers(MaxViewportCount, buffers);

		for (size_t i = 0; i < MaxViewportCount; ++i)
		{
			viewportData[i].uniformBlockObject = buffers[i];
			device->BindBuffer(RenderBufferTarget::UniformBuffer, viewportData[i].uniformBlockObject);
			device->SetBufferData(RenderBufferTarget::UniformBuffer, sizeof(ViewportUniformBlock), nullptr, RenderBufferUsage::DynamicDraw);
		}
	}

	{
		// Create geometry framebuffer and textures

		framebufferCount += 1;

		RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];
		gbuffer.width = frameBufferSizei.x;
		gbuffer.height = frameBufferSizei.y;

		// Create and bind framebuffer

		device->CreateFramebuffers(1, &gbuffer.framebuffer);
		device->BindFramebuffer(RenderFramebufferTarget::Framebuffer, gbuffer.framebuffer);

		unsigned int gbufferTextureCount = 4;
		device->CreateTextures(gbufferTextureCount, &framebufferTextures[framebufferTextureCount]);

		gBufferAlbedoTextureIndex = framebufferTextureCount++;
		gBufferNormalTextureIndex = framebufferTextureCount++;
		gBufferMaterialTextureIndex = framebufferTextureCount++;
		fullscreenDepthTextureIndex = framebufferTextureCount++;

		RenderFramebufferAttachment colAtt[3] = {
			RenderFramebufferAttachment::Color0,
			RenderFramebufferAttachment::Color1,
			RenderFramebufferAttachment::Color2
		};

		// Albedo color buffer

		unsigned int albTexture = framebufferTextures[gBufferAlbedoTextureIndex];
		device->BindTexture(RenderTextureTarget::Texture2d, albTexture);

		RenderCommandData::SetTextureImage2D albTextureImage{
			RenderTextureTarget::Texture2d, 0, GL_SRGB8, gbuffer.width, gbuffer.height, GL_RGB, GL_UNSIGNED_BYTE, nullptr
		};
		device->SetTextureImage2D(&albTextureImage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);

		RenderCommandData::AttachFramebufferTexture2D albAttachTexture{
			RenderFramebufferTarget::Framebuffer, colAtt[0], RenderTextureTarget::Texture2d, albTexture, 0
		};
		device->AttachFramebufferTexture2D(&albAttachTexture);

		// Normal buffer

		unsigned int norTexture = framebufferTextures[gBufferNormalTextureIndex];
		device->BindTexture(RenderTextureTarget::Texture2d, norTexture);

		RenderCommandData::SetTextureImage2D norTextureImage{
			RenderTextureTarget::Texture2d, 0, GL_RG16, gbuffer.width, gbuffer.height, GL_RG, GL_UNSIGNED_SHORT, nullptr
		};
		device->SetTextureImage2D(&norTextureImage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);

		RenderCommandData::AttachFramebufferTexture2D norAttachTexture{
			RenderFramebufferTarget::Framebuffer, colAtt[1], RenderTextureTarget::Texture2d, norTexture, 0
		};
		device->AttachFramebufferTexture2D(&norAttachTexture);

		// Emissivity buffer

		unsigned int matTexture = framebufferTextures[gBufferMaterialTextureIndex];
		device->BindTexture(RenderTextureTarget::Texture2d, matTexture);

		RenderCommandData::SetTextureImage2D matTextureImage{
			RenderTextureTarget::Texture2d, 0, GL_RGB8, gbuffer.width, gbuffer.height, GL_RGB, GL_UNSIGNED_BYTE, nullptr
		};
		device->SetTextureImage2D(&matTextureImage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);

		RenderCommandData::AttachFramebufferTexture2D matAttachTexture{
			RenderFramebufferTarget::Framebuffer, colAtt[2], RenderTextureTarget::Texture2d, matTexture, 0
		};
		device->AttachFramebufferTexture2D(&matAttachTexture);

		// Which color attachments we'll use for rendering
		device->SetFramebufferDrawBuffers(sizeof(colAtt) / sizeof(colAtt[0]), colAtt);

		// Create and attach depth buffer
		unsigned int depthTexture = framebufferTextures[fullscreenDepthTextureIndex];
		device->BindTexture(RenderTextureTarget::Texture2d, depthTexture);

		RenderCommandData::SetTextureImage2D depthTextureImage{
			RenderTextureTarget::Texture2d, 0, GL_DEPTH_COMPONENT, gbuffer.width, gbuffer.height, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
		};
		device->SetTextureImage2D(&depthTextureImage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);

		// Set depth texture to clamp to edge, because SSAO pass can read beyond the edge
		device->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		device->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);

		RenderCommandData::AttachFramebufferTexture2D depthAttachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Depth,
			RenderTextureTarget::Texture2d, depthTexture, 0
		};
		device->AttachFramebufferTexture2D(&depthAttachTexture);

		framebufferTextureCount += gbufferTextureCount;
	}

	{
		// Create shadow framebuffer

		int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
		unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
		Vec2i size(shadowSide * shadowCascadeCount, shadowSide);

		framebufferCount += 1;

		RendererFramebuffer& framebuffer = framebufferData[FramebufferIndexShadow];
		framebuffer.width = size.x;
		framebuffer.height = size.y;

		// Create and bind framebuffer

		device->CreateFramebuffers(1, &framebuffer.framebuffer);
		device->BindFramebuffer(RenderFramebufferTarget::Framebuffer, framebuffer.framebuffer);

		// We aren't rendering to any color attachments
		RenderFramebufferAttachment drawBuffers = RenderFramebufferAttachment::None;
		device->SetFramebufferDrawBuffers(1, &drawBuffers);

		// Create texture
		device->CreateTextures(1, &framebufferTextures[framebufferTextureCount]);
		
		shadowDepthTextureIndex = framebufferTextureCount++;

		// Create and attach depth buffer
		unsigned int depthTexture = framebufferTextures[shadowDepthTextureIndex];
		device->BindTexture(RenderTextureTarget::Texture2d, depthTexture);

		RenderCommandData::SetTextureImage2D depthTextureImage{
			RenderTextureTarget::Texture2d, 0, GL_DEPTH_COMPONENT, size.x, size.y, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
		};
		device->SetTextureImage2D(&depthTextureImage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Linear);
		device->SetTextureWrapModeU(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		device->SetTextureWrapModeV(RenderTextureTarget::Texture2d, RenderTextureWrapMode::ClampToEdge);
		device->SetTextureCompareMode(RenderTextureTarget::Texture2d, RenderTextureCompareMode::CompareRefToTexture);
		device->SetTextureCompareFunc(RenderTextureTarget::Texture2d, RenderTextureCompareFunc::LessThanOrEqual);

		RenderCommandData::AttachFramebufferTexture2D depthFramebufferTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Depth,
			RenderTextureTarget::Texture2d, depthTexture, 0
		};
		device->AttachFramebufferTexture2D(&depthFramebufferTexture);

		// Clear texture and framebuffer binds
		device->BindTexture(RenderTextureTarget::Texture2d, 0);
		device->BindFramebuffer(RenderFramebufferTarget::Framebuffer, 0);
	}

	{
		// HDR light accumulation framebuffer

		framebufferCount += 1;

		RendererFramebuffer& framebuffer = framebufferData[FramebufferIndexLightAcc];
		framebuffer.width = frameBufferSizei.x;
		framebuffer.height = frameBufferSizei.y;

		// Create and bind framebuffer

		device->CreateFramebuffers(1, &framebuffer.framebuffer);
		device->BindFramebuffer(RenderFramebufferTarget::Framebuffer, framebuffer.framebuffer);

		device->CreateTextures(1, &framebufferTextures[framebufferTextureCount]);
		lightAccumulationTextureIndex = framebufferTextureCount++;
		unsigned int lightAccTexture = framebufferTextures[lightAccumulationTextureIndex];

		device->BindTexture(RenderTextureTarget::Texture2d, lightAccTexture);

		RenderCommandData::SetTextureStorage2D storage{
			RenderTextureTarget::Texture2d, 1, RenderTextureSizedFormat::RGB16F, framebuffer.width, framebuffer.height
		};
		device->SetTextureStorage2D(&storage);

		device->SetTextureMinFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);
		device->SetTextureMagFilter(RenderTextureTarget::Texture2d, RenderTextureFilterMode::Nearest);

		RenderCommandData::AttachFramebufferTexture2D colorAttachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Color0,
			RenderTextureTarget::Texture2d, lightAccTexture, 0
		};
		device->AttachFramebufferTexture2D(&colorAttachTexture);

		// Reuse depth texture from gbuffer
		unsigned int depthTexture = framebufferTextures[fullscreenDepthTextureIndex];
		RenderCommandData::AttachFramebufferTexture2D depthAttachTexture{
			RenderFramebufferTarget::Framebuffer, RenderFramebufferAttachment::Depth,
			RenderTextureTarget::Texture2d, depthTexture, 0
		};
		device->AttachFramebufferTexture2D(&depthAttachTexture);
	}
	
	{
		// Set up uniform buffer for tonemapping pass

		device->CreateBuffers(1, &tonemapUniformBufferId);
		device->BindBuffer(RenderBufferTarget::UniformBuffer, tonemapUniformBufferId);
		device->SetBufferData(RenderBufferTarget::UniformBuffer, sizeof(TonemapUniformBlock), nullptr, RenderBufferUsage::DynamicDraw);
	}

	{
		// Create opaque lighting pass uniform buffer

		device->CreateBuffers(1, &lightingUniformBufferId);
		device->BindBuffer(RenderBufferTarget::UniformBuffer, lightingUniformBufferId);
		device->SetBufferData(RenderBufferTarget::UniformBuffer, sizeof(LightingUniformBlock), nullptr, RenderBufferUsage::DynamicDraw);
	}

	{
		// Create screen filling quad
		fullscreenMesh = meshManager->CreateMesh();
		MeshPresets::UploadPlane(meshManager, fullscreenMesh);
	}
	
	{
		const char* path = "res/materials/deferred_geometry/shadow_depth.material.json";
		shadowMaterial = materialManager->GetIdByPath(StringRef(path));
	}

	{
		const char* path = "res/shaders/deferred_lighting/lighting.shader.json";
		lightingShaderId = shaderManager->GetIdByPath(StringRef(path));
	}

	{
		const char* path = "res/shaders/post_process/tonemap.shader.json";
		tonemappingShaderId = shaderManager->GetIdByPath(StringRef(path));
	}

	// Create skybox entity
	{
		EntityManager* em = Engine::GetInstance()->GetEntityManager();
		skyboxEntity = em->Create();

		RenderObjectId skyboxRenderObj = AddRenderObject(skyboxEntity);

		MeshId skyboxMesh = meshManager->CreateMesh();
		MeshPresets::UploadCube(meshManager, skyboxMesh);
		SetMeshId(skyboxRenderObj, skyboxMesh);

		// Expand skybox extents to make sure it is always rendered
		BoundingBox skyboxBounds;
		skyboxBounds.extents = Vec3f(1e9, 1e9, 1e9);
		data.bounds[skyboxRenderObj.i] = skyboxBounds;
	}
}

void Renderer::Deinitialize()
{
	if (fullscreenMesh.IsValid())
	{
		meshManager->RemoveMesh(fullscreenMesh);
		fullscreenMesh = MeshId{ 0 };
	}

	if (lightingUniformBufferId != 0)
	{
		device->DestroyBuffers(1, &lightingUniformBufferId);
		lightingUniformBufferId = 0;
	}

	if (objectUniformBuffers.GetCount() > 0)
	{
		device->DestroyBuffers(objectUniformBuffers.GetCount(), objectUniformBuffers.GetData());
		objectUniformBuffers.Clear();
	}

	if (framebufferData != nullptr)
	{
		for (unsigned int i = 0; i < framebufferCount; ++i)
		{
			RendererFramebuffer& fb = framebufferData[i];

			if (fb.framebuffer != 0)
			{
				device->DestroyFramebuffers(1, &fb.framebuffer);

				fb.framebuffer = 0;
			}
		}

		this->allocator->Deallocate(framebufferData);
		framebufferData = nullptr;
		framebufferCount = 0;
	}

	if (framebufferTextures != nullptr)
	{
		device->DestroyTextures(framebufferTextureCount, framebufferTextures);

		this->allocator->Deallocate(framebufferTextures);
		framebufferTextures = nullptr;
		framebufferTextureCount = 0;
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

		this->allocator->Deallocate(viewportData);
		viewportData = nullptr;
		viewportCount = 0;
	}
}

void Renderer::Render(Scene* scene)
{
	unsigned int objectDrawCount = PopulateCommandList(scene);
	UpdateUniformBuffers(objectDrawCount);

	int objectDrawsProcessed = 0;

	unsigned int lastVpIdx = MaxViewportCount;
	unsigned int lastShaderProgram = 0;
	MeshDrawData* draw = nullptr;
	MeshId lastMeshId = MeshId{ 0 };
	MaterialId lastMaterialId = MaterialId{ 0 };

	TransformUniformBlock objectUniforms;

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;

		// If command is not control command, draw object
		if (ParseControlCommand(command) == false)
		{
			unsigned int mat = renderOrder.materialId.GetValue(command);
			unsigned int vpIdx = renderOrder.viewportIndex.GetValue(command);
			const RenderViewport& viewport = viewportData[vpIdx];

			if (mat != RenderOrderConfiguration::CallbackMaterialId)
			{
				MaterialId matId = MaterialId{mat};
				unsigned int objIdx = renderOrder.renderObject.GetValue(command);

				// Update viewport uniform block
				if (vpIdx != lastVpIdx)
				{
					unsigned int ubo = viewportData[vpIdx].uniformBlockObject;
					device->BindBufferBase(RenderBufferTarget::UniformBuffer, ViewportUniformBlock::BindingPoint, ubo);

					lastVpIdx = vpIdx;
				}

				if (matId != lastMaterialId)
				{
					lastMaterialId = matId;
					const MaterialData& matData = materialManager->GetMaterialData(matId);

					if (matData.cachedShaderDeviceId != lastShaderProgram)
					{
						device->UseShaderProgram(matData.cachedShaderDeviceId);
						lastShaderProgram = matData.cachedShaderDeviceId;
					}

					BindMaterialTextures(matData);

					// Bind material uniform block to shader
					device->BindBufferBase(RenderBufferTarget::UniformBuffer, MaterialUniformBlock::BindingPoint, matData.uniformBufferObject);
				}

				// Bind object transform uniform block to shader

				int bufferIndex = objectDrawsProcessed / objectsPerUniformBuffer;
				int objectInBuffer = objectDrawsProcessed % objectsPerUniformBuffer;

				RenderCommandData::BindBufferRange bind{
					RenderBufferTarget::UniformBuffer, TransformUniformBlock::BindingPoint,
					objectUniformBuffers[bufferIndex], objectInBuffer * objectUniformBlockStride, objectUniformBlockStride
				};

				device->BindBufferRange(&bind);

				MeshId mesh = data.mesh[objIdx];

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
				unsigned int callbackId = renderOrder.renderObject.GetValue(command);

				if (callbackId > 0 && callbackId <= customRenderers.GetCount())
				{
					CustomRenderer* customRenderer = customRenderers[callbackId - 1];

					if (customRenderer != nullptr)
					{
						CustomRenderer::RenderParams params;
						params.viewport = &viewport;
						params.callbackId = callbackId;
						params.command = command;
						params.scene = scene;

						customRenderer->RenderCustom(params);

						// Reset state cache
						lastVpIdx = MaxViewportCount;
						lastShaderProgram = 0;
						draw = nullptr;
						lastMeshId = MeshId{ 0 };
						lastMaterialId = MaterialId{ 0 };
					}
				}
			}
		}
	}

	commandList.Clear();
}

void Renderer::BindMaterialTextures(const MaterialData& material) const
{
	unsigned int usedTextures = 0;

	for (unsigned uIndex = 0; uIndex < material.uniforms.textureUniformCount; ++uIndex)
	{
		const TextureUniform& u = material.uniforms.textureUniforms[uIndex];

		switch (u.type)
		{
		case UniformDataType::Tex2D:
		case UniformDataType::TexCube:
			device->SetActiveTextureUnit(usedTextures);
			device->BindTexture(u.textureTarget, u.textureName);
			device->SetUniformInt(u.uniformLocation, usedTextures);
			++usedTextures;
			break;

		default:
			break;
		}
	}
}

void Renderer::BindTextures(const ShaderData& shader, unsigned int count, uint32_t* nameHashes, unsigned int* textures)
{
	for (unsigned int i = 0; i < count; ++i)
	{
		const TextureUniform* tu = shader.uniforms.FindTextureUniformByNameHash(nameHashes[i]);
		if (tu != nullptr)
		{
			device->SetUniformInt(tu->uniformLocation, i);
			device->SetActiveTextureUnit(i);
			device->BindTexture(RenderTextureTarget::Texture2d, textures[i]);
		}
	}
}

void Renderer::RenderCustom(const CustomRenderer::RenderParams& params)
{
	if (params.callbackId == deferredLightingCallback)
		RenderDeferredLighting(params);
	else if (params.callbackId == tonemappingCallback)
		RenderTonemapping(params);
}

void Renderer::RenderDeferredLighting(const CustomRenderer::RenderParams& params)
{
	// Both SSAO and deferred lighting passes use these

	Scene* scene = params.scene;
	ProjectionParameters projParams = scene->GetActiveCamera()->parameters;

	device->DepthTestDisable();

	MeshDrawData* meshDrawData = meshManager->GetDrawData(fullscreenMesh);
	device->BindVertexArray(meshDrawData->vertexArrayObject);

	unsigned int ssaoBindingPoint = ssao->GetUniformBufferBindingPoint();

	// SSAO occlusion pass

	ssao->UpdateOcclusionUniformBuffer(projParams);

	const ScreenSpaceAmbientOcclusion::PassInfo& occlusionPass = ssao->GetOcclusionPassInfo();

	RenderCommandData::BindFramebufferData bindFramebufferCommand;
	bindFramebufferCommand.target = RenderFramebufferTarget::Framebuffer;
	bindFramebufferCommand.framebuffer = occlusionPass.framebufferId;
	device->BindFramebuffer(&bindFramebufferCommand);

	const ShaderData& ssaoOcclusionShader = shaderManager->GetShaderData(occlusionPass.shaderId);

	device->UseShaderProgram(ssaoOcclusionShader.driverId);

	{
		uint32_t textureNameHashes[] = { "g_normal"_hash, "g_depth"_hash, "noise_texture"_hash };
		unsigned int textureIds[] = {
			framebufferTextures[gBufferNormalTextureIndex],
			framebufferTextures[fullscreenDepthTextureIndex],
			ssao->GetNoiseTextureId()
		};
		BindTextures(ssaoOcclusionShader, 3, textureNameHashes, textureIds);
	}

	device->BindBufferBase(RenderBufferTarget::UniformBuffer, ssaoBindingPoint, occlusionPass.uniformBufferId);

	device->DrawIndexed(meshDrawData->primitiveMode, meshDrawData->count, meshDrawData->indexType);

	// SSAO blur pass

	ssao->UpdateBlurUniformBuffer();

	const ScreenSpaceAmbientOcclusion::PassInfo& blurPass = ssao->GetBlurPassInfo();

	bindFramebufferCommand.target = RenderFramebufferTarget::Framebuffer;
	bindFramebufferCommand.framebuffer = blurPass.framebufferId;
	device->BindFramebuffer(&bindFramebufferCommand);

	const ShaderData& ssaoBlurShader = shaderManager->GetShaderData(blurPass.shaderId);

	device->UseShaderProgram(ssaoBlurShader.driverId);
	
	{
		uint32_t textureNameHashes[] = { "occlusion_map"_hash };
		unsigned int textureIds[] = { ssao->GetOcclusionPassInfo().textureId };
		BindTextures(ssaoBlurShader, 1, textureNameHashes, textureIds);
	}

	device->BindBufferBase(RenderBufferTarget::UniformBuffer, ssaoBindingPoint, blurPass.uniformBufferId);

	device->DrawIndexed(meshDrawData->primitiveMode, meshDrawData->count, meshDrawData->indexType);

	// Deferred lighting

	// Bind HDR light accumulation framebuffer
	bindFramebufferCommand.target = RenderFramebufferTarget::Framebuffer;
	bindFramebufferCommand.framebuffer = framebufferData[FramebufferIndexLightAcc].framebuffer;
	device->BindFramebuffer(&bindFramebufferCommand);

	device->Clear(GL_COLOR_BUFFER_BIT);

	const ShaderData& shader = shaderManager->GetShaderData(lightingShaderId);

	device->UseShaderProgram(shader.driverId);

	{
		uint32_t textureNameHashes[] = {
			"g_albedo"_hash, "g_normal"_hash, "g_material"_hash, "g_depth"_hash,
			"ssao_map"_hash, "shadow_map"_hash
		};

		unsigned int textureIds[] = {
			framebufferTextures[gBufferAlbedoTextureIndex],
			framebufferTextures[gBufferNormalTextureIndex],
			framebufferTextures[gBufferMaterialTextureIndex],
			framebufferTextures[fullscreenDepthTextureIndex],
			ssao->GetBlurPassInfo().textureId,
			framebufferTextures[shadowDepthTextureIndex]
		};

		BindTextures(shader, 6, textureNameHashes, textureIds);
	}

	// Update lightingUniformBuffer data
	LightingUniformBlock lightingUniforms;
	UpdateLightingDataToUniformBuffer(projParams, scene, lightingUniforms);

	device->BindBuffer(RenderBufferTarget::UniformBuffer, lightingUniformBufferId);
	device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(LightingUniformBlock), &lightingUniforms);
	device->BindBufferBase(RenderBufferTarget::UniformBuffer, LightingUniformBlock::BindingPoint, lightingUniformBufferId);

	// Draw fullscreen quad
	device->DrawIndexed(meshDrawData->primitiveMode, meshDrawData->count, meshDrawData->indexType);
}

void Renderer::RenderTonemapping(const CustomRenderer::RenderParams& params)
{
	device->BlendingDisable();
	device->DepthTestDisable();

	MeshDrawData* meshDrawData = meshManager->GetDrawData(fullscreenMesh);
	device->BindVertexArray(meshDrawData->vertexArrayObject);

	// Bind default framebuffer

	RenderCommandData::BindFramebufferData bindFramebufferCommand;
	bindFramebufferCommand.target = RenderFramebufferTarget::Framebuffer;
	bindFramebufferCommand.framebuffer = 0;
	device->BindFramebuffer(&bindFramebufferCommand);

	const ShaderData& shader = shaderManager->GetShaderData(tonemappingShaderId);
	device->UseShaderProgram(shader.driverId);

	TonemapUniformBlock uniforms;
	uniforms.exposure = 1.0f;

	device->BindBuffer(RenderBufferTarget::UniformBuffer, tonemapUniformBufferId);
	device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(TonemapUniformBlock), &uniforms);
	device->BindBufferBase(RenderBufferTarget::UniformBuffer, 0, tonemapUniformBufferId);

	{
		uint32_t textureNameHashes[] = { "light_acc_map"_hash };
		unsigned int textureIds[] = { framebufferTextures[lightAccumulationTextureIndex] };
		BindTextures(shader, 1, textureNameHashes, textureIds);
	}

	device->DrawIndexed(meshDrawData->primitiveMode, meshDrawData->count, meshDrawData->indexType);
}

void Renderer::UpdateLightingDataToUniformBuffer(
	const ProjectionParameters& projection, const Scene* scene, LightingUniformBlock& uniformsOut)
{
	const RenderViewport& fsvp = viewportData[viewportIndexFullscreen];

	// Update directional light viewports
	Array<LightId>& directionalLights = lightResultArray;
	lightManager->GetDirectionalLights(directionalLights);

	Vec2f halfNearPlane;
	halfNearPlane.y = std::tan(projection.height * 0.5f);
	halfNearPlane.x = halfNearPlane.y * projection.aspect;
	uniformsOut.halfNearPlane = halfNearPlane;

	int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
	unsigned int cascadeCount = CascadedShadowMap::GetCascadeCount();
	uniformsOut.shadowMapScale = Vec2f(1.0f / (cascadeCount * shadowSide), 1.0f / shadowSide);

	const RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];
	uniformsOut.frameResolution = Vec2f(gbuffer.width, gbuffer.height);

	// Set the perspective matrix
	uniformsOut.perspectiveMatrix = fsvp.projection;

	// Directional light
	if (directionalLights.GetCount() > 0)
	{
		LightId dirLightId = directionalLights[0];

		Mat3x3f orientation = lightManager->GetOrientation(dirLightId);
		Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
		Vec4f vLightDir = fsvp.view * Vec4f(wLightDir, 0.0f);
		uniformsOut.lightDirections[0] = vLightDir;

		Vec3f lightCol = lightManager->GetColor(dirLightId);
		uniformsOut.lightColors[0] = lightCol;
	}

	lightResultArray.Clear();
	Array<LightId>& nonDirLights = lightResultArray;
	lightManager->GetNonDirectionalLightsWithinFrustum(fsvp.frustum, nonDirLights);

	// Count the different light types

	unsigned int pointLightCount = 0;
	unsigned int spotLightCount = 0;

	for (unsigned int lightIdx = 0, count = nonDirLights.GetCount(); lightIdx < count; ++lightIdx)
	{
		LightType type = lightManager->GetLightType(nonDirLights[lightIdx]);
		if (type == LightType::Point)
			pointLightCount += 1;
		else if (type == LightType::Spot)
			spotLightCount += 1;
	}

	uniformsOut.pointLightCount = pointLightCount;
	uniformsOut.spotLightCount = spotLightCount;

	const unsigned int dirLightOffset = 1;
	unsigned int pointLightsAdded = 0;
	unsigned int spotLightsAdded = 0;

	// Light other visible lights
	for (unsigned int lightIdx = 0, count = nonDirLights.GetCount(); lightIdx < count; ++lightIdx)
	{
		unsigned int shaderLightIdx;

		LightId lightId = nonDirLights[lightIdx];

		// Point lights come first, so offset spot lights with the amount of point lights
		LightType type = lightManager->GetLightType(lightId);
		if (type == LightType::Spot)
		{
			shaderLightIdx = dirLightOffset + pointLightCount + spotLightsAdded;
			spotLightsAdded += 1;

			Mat3x3f orientation = lightManager->GetOrientation(lightId);
			Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
			Vec4f vLightDir = fsvp.view * Vec4f(wLightDir, 0.0f);
			vLightDir.w = lightManager->GetSpotAngle(lightId);
			uniformsOut.lightDirections[shaderLightIdx] = vLightDir;
		}
		else
		{
			shaderLightIdx = dirLightOffset + pointLightsAdded;
			pointLightsAdded += 1;
		}

		Vec3f wLightPos = lightManager->GetPosition(lightId);
		Vec3f vLightPos = (fsvp.view * Vec4f(wLightPos, 1.0f)).xyz();
		uniformsOut.lightPositions[shaderLightIdx] = vLightPos;

		Vec3f lightCol = lightManager->GetColor(lightId);
		uniformsOut.lightColors[shaderLightIdx] = lightCol;
	}

	lightResultArray.Clear();

	// shadow_params.splits[0] is the near depth
	uniformsOut.shadowSplits[0] = projection.near;

	unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
	uniformsOut.cascadeCount = shadowCascadeCount;

	float cascadeSplitDepths[CascadedShadowMap::MaxCascadeCount];
	CascadedShadowMap::CalculateSplitDepths(projection, cascadeSplitDepths);

	Mat4x4f bias;
	bias[0] = 0.5; bias[1] = 0.0; bias[2] = 0.0; bias[3] = 0.0;
	bias[4] = 0.0; bias[5] = 0.5; bias[6] = 0.0; bias[7] = 0.0;
	bias[8] = 0.0; bias[9] = 0.0; bias[10] = 0.5; bias[11] = 0.0;
	bias[12] = 0.5; bias[13] = 0.5; bias[14] = 0.5; bias[15] = 1.0;

	// Update transforms and split depths for each shadow cascade
	for (size_t vpIdx = 0; vpIdx < shadowCascadeCount; ++vpIdx)
	{
		Mat4x4f viewToLight = viewportData[vpIdx].viewProjection * fsvp.viewToWorld;
		Mat4x4f shadowMat = bias * viewToLight;

		uniformsOut.shadowMatrices[vpIdx] = shadowMat;
		uniformsOut.shadowSplits[vpIdx + 1] = cascadeSplitDepths[vpIdx];
	}

	Vec3f ambientColor(scene->ambientColor.r, scene->ambientColor.g, scene->ambientColor.b);
	uniformsOut.ambientColor = ambientColor;

	uniformsOut.shadowBiasOffset = 0.001f;
	uniformsOut.shadowBiasFactor = 0.0019f;
	uniformsOut.shadowBiasClamp = 0.01f;
}

void Renderer::UpdateUniformBuffers(unsigned int objectDrawCount)
{
	unsigned int buffersRequired = (objectDrawCount + objectsPerUniformBuffer - 1) / objectsPerUniformBuffer;

	// Create new object transform uniform buffers if needed
	if (buffersRequired > objectUniformBuffers.GetCount())
	{
		unsigned int currentCount = objectUniformBuffers.GetCount();
		objectUniformBuffers.Resize(buffersRequired);

		unsigned int addCount = buffersRequired - currentCount;
		device->CreateBuffers(addCount, objectUniformBuffers.GetData() + currentCount);

		for (unsigned int i = currentCount; i < buffersRequired; ++i)
		{
			device->BindBuffer(RenderBufferTarget::UniformBuffer, objectUniformBuffers[i]);

			RenderCommandData::SetBufferStorage setStorage{};
			setStorage.target = RenderBufferTarget::UniformBuffer;
			setStorage.size = ObjectUniformBufferSize;
			setStorage.dynamicStorage = true;
			setStorage.mapWriteAccess = true;

			device->SetBufferStorage(&setStorage);
		}
	}

	int mappedBufferIndex = -1;
	char* mappedBuffer = nullptr;
	int objectDrawsProcessed = 0;

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;
		unsigned int mat = renderOrder.materialId.GetValue(command);
		unsigned int vpIdx = renderOrder.viewportIndex.GetValue(command);
		unsigned int objIdx = renderOrder.renderObject.GetValue(command);

		// Is regular draw command
		if (IsDrawCommand(command) && mat != RenderOrderConfiguration::CallbackMaterialId)
		{
			int bufferIndex = objectDrawsProcessed / objectsPerUniformBuffer;
			int objectInBuffer = objectDrawsProcessed % objectsPerUniformBuffer;

			if (bufferIndex != mappedBufferIndex)
			{
				if (mappedBufferIndex >= 0)
					device->UnmapBuffer(RenderBufferTarget::UniformBuffer);

				device->BindBuffer(RenderBufferTarget::UniformBuffer, objectUniformBuffers[bufferIndex]);

				RenderCommandData::MapBufferRange map{};
				map.target = RenderBufferTarget::UniformBuffer;
				map.offset = 0;
				map.length = ObjectUniformBufferSize;
				map.writeAccess = true;
				map.invalidateBuffer = true;

				mappedBuffer = static_cast<char*>(device->MapBufferRange(&map));
				mappedBufferIndex = bufferIndex;
			}

			TransformUniformBlock* tu = reinterpret_cast<TransformUniformBlock*>(mappedBuffer + objectUniformBlockStride * objectInBuffer);

			const Mat4x4f& model = data.transform[objIdx];
			tu->MVP = viewportData[vpIdx].viewProjection * model;
			tu->MV = viewportData[vpIdx].view * model;
			tu->M = model;

			objectDrawsProcessed += 1;
		}
	}

	if (mappedBufferIndex >= 0)
		device->UnmapBuffer(RenderBufferTarget::UniformBuffer);
}

bool Renderer::IsDrawCommand(uint64_t orderKey)
{
	return renderOrder.command.GetValue(orderKey) == static_cast<uint64_t>(RenderCommandType::Draw);
}

bool Renderer::ParseControlCommand(uint64_t orderKey)
{
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
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* blendFn = reinterpret_cast<RenderCommandData::BlendFunctionData*>(data);
			device->BlendFunction(blendFn);

		}
			break;

		case RenderControlType::Viewport:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* viewport = reinterpret_cast<RenderCommandData::ViewportData*>(data);
			device->Viewport(viewport);
		}
			break;

		case RenderControlType::DepthRange:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
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
			unsigned int fn = renderOrder.commandData.GetValue(orderKey);
			device->DepthTestFunction(fn);
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
			unsigned int mask = renderOrder.commandData.GetValue(orderKey);
			device->Clear(mask);
		}
			break;

		case RenderControlType::ClearColor:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* color = reinterpret_cast<RenderCommandData::ClearColorData*>(data);
			device->ClearColor(color);
		}
			break;

		case RenderControlType::ClearDepth:
		{
			unsigned int intDepth = renderOrder.commandData.GetValue(orderKey);
			float depth = *reinterpret_cast<float*>(&intDepth);
			device->ClearDepth(depth);
		}
			break;

		case RenderControlType::BindFramebuffer:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
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

unsigned int Renderer::PopulateCommandList(Scene* scene)
{
	const float mainViewportMinObjectSize = 50.0f;
	const float shadowViewportMinObjectSize = 30.0f;

	// Get camera transforms

	Camera* renderCamera = scene->GetActiveCamera();
	const ProjectionParameters& projectionParams = renderCamera->parameters;
	SceneObjectId cameraObject = scene->Lookup(renderCamera->GetEntity());
	Mat4x4f cameraTransform = scene->GetWorldTransform(cameraObject);

	Mat4x4f cullingCameraTransform = lockCullingCamera ? lockCullingCameraTransform : cameraTransform;

	if (lockCullingCamera == false)
		lockCullingCameraTransform = cullingCameraTransform;

	Vec3f cameraPos = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
	unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
	Vec2i shadowCascadeSize(shadowSide, shadowSide);
	Vec2i shadowTextureSize(shadowSide * shadowCascadeCount, shadowSide);
	Mat4x4f cascadeViewTransforms[CascadedShadowMap::MaxCascadeCount];
	ProjectionParameters lightProjections[CascadedShadowMap::MaxCascadeCount];

	// Update skybox parameters
	{
		RenderObjectId skyboxRenderObj = Lookup(skyboxEntity);

		RenderOrderData order;
		order.material = scene->GetSkyboxMaterial();
		order.transparency = TransparencyType::Skybox;
		SetOrderData(skyboxRenderObj, order);

		Mat4x4f skyboxTransform = Mat4x4f::Translate(cameraPos);
		data.transform[skyboxRenderObj.i] = skyboxTransform;
	}

	// Reset the used viewport count
	viewportCount = 0;

	// Update directional light viewports
	lightManager->GetDirectionalLights(lightResultArray);

	ViewportUniformBlock viewportUniforms;
	
	for (unsigned int i = 0, count = lightResultArray.GetCount(); i < count; ++i)
	{
		LightId id = lightResultArray[i];

		if (lightManager->GetShadowCasting(id) &&
			viewportCount + shadowCascadeCount + 1 <= MaxViewportCount)
		{
			Mat3x3f orientation = lightManager->GetOrientation(id);
			Vec3f lightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);

			CascadedShadowMap::CalculateCascadeFrusta(lightDir, cameraTransform, projectionParams, cascadeViewTransforms, lightProjections);

			for (unsigned int cascade = 0; cascade < shadowCascadeCount; ++cascade)
			{
				unsigned int vpIdx = viewportCount;
				viewportCount += 1;

				RenderViewport& vp = viewportData[vpIdx];
				vp.position = (cascadeViewTransforms[cascade] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
				vp.forward = (cascadeViewTransforms[cascade] * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
				vp.farMinusNear = lightProjections[cascade].far - lightProjections[cascade].near;
				vp.minusNear = -lightProjections[cascade].near;
				vp.objectMinScreenSizePx = shadowViewportMinObjectSize;
				vp.viewToWorld = cascadeViewTransforms[cascade];
				vp.view = vp.viewToWorld.GetInverse();
				vp.projection = lightProjections[cascade].GetProjectionMatrix();
				vp.viewProjection = vp.projection * vp.view;
				vp.viewportRectangle.size = shadowCascadeSize;
				vp.viewportRectangle.position = Vec2i(cascade * shadowSide, 0);
				vp.frustum.Update(lightProjections[cascade], cascadeViewTransforms[cascade]);
				vp.framebufferIndex = FramebufferIndexShadow;

				viewportUniforms.VP = vp.viewProjection;
				viewportUniforms.V = vp.view;
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

		const RendererFramebuffer& fb = framebufferData[FramebufferIndexGBuffer];

		RenderViewport& vp = viewportData[vpIdx];
		vp.position = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
		vp.forward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
		vp.farMinusNear = renderCamera->parameters.far - renderCamera->parameters.near;
		vp.minusNear = -renderCamera->parameters.near;
		vp.objectMinScreenSizePx = mainViewportMinObjectSize;
		vp.viewToWorld = cameraTransform;
		vp.view = Camera::GetViewMatrix(vp.viewToWorld);
		vp.projection = projectionParams.GetProjectionMatrix();
		vp.viewProjection = vp.projection * vp.view;
		vp.viewportRectangle.size = Vec2i(fb.width, fb.height);
		vp.viewportRectangle.position = Vec2i(0, 0);
		vp.frustum.Update(renderCamera->parameters, cullingCameraTransform);
		vp.framebufferIndex = FramebufferIndexGBuffer;

		viewportUniforms.VP = vp.viewProjection;
		viewportUniforms.V = vp.view;
		viewportUniforms.P = vp.projection;

		device->BindBuffer(RenderBufferTarget::UniformBuffer, vp.uniformBlockObject);
		device->SetBufferSubData(RenderBufferTarget::UniformBuffer, 0, sizeof(ViewportUniformBlock), &viewportUniforms);

		this->viewportIndexFullscreen = vpIdx;
	}

	const FrustumPlanes& fullscreenFrustum = viewportData[viewportIndexFullscreen].frustum;

	unsigned int colorAndDepthMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

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
	commandList.AddControl(0, g_pass, 1, ctrl::DepthTestFunction, GL_LESS);

	// Enable depth writing
	commandList.AddControl(0, g_pass, 2, ctrl::DepthWriteEnable);

	// Set face culling on
	commandList.AddControl(0, g_pass, 3, ctrl::CullFaceEnable);

	// Set face culling to cull back faces
	commandList.AddControl(0, g_pass, 4, ctrl::CullFaceBack);

	{
		// Set clear depth
		float depth = 1.0f;
		unsigned int* intDepthPtr = reinterpret_cast<unsigned int*>(&depth);

		commandList.AddControl(0, g_pass, 5, ctrl::ClearDepth, *intDepthPtr);
	}

	// Disable blending
	commandList.AddControl(0, g_pass, 6, ctrl::BlendingDisable);

	{
		// Bind shadow framebuffer before any shadow cascade draws
		RenderCommandData::BindFramebufferData data;
		data.target = RenderFramebufferTarget::Framebuffer;
		data.framebuffer = framebufferData[FramebufferIndexShadow].framebuffer;

		commandList.AddControl(0, g_pass, 7, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	{
		const RendererFramebuffer& fb = framebufferData[FramebufferIndexShadow];

		// Set viewport size to full framebuffer size before clearing
		RenderCommandData::ViewportData data;
		data.x = 0;
		data.y = 0;
		data.w = fb.width;
		data.h = fb.height;

		commandList.AddControl(0, g_pass, 8, ctrl::Viewport, sizeof(data), &data);
	}

	// Clear shadow framebuffer RenderFramebufferTarget::Framebuffer
	commandList.AddControl(0, g_pass, 9, ctrl::Clear, GL_DEPTH_BUFFER_BIT);

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

		commandList.AddControl(vpIdx, g_pass, 10, ctrl::Viewport, sizeof(data), &data);
	}

	// Enable sRGB conversion for framebuffer
	commandList.AddControl(0, g_pass, 11, ctrl::FramebufferSrgbEnable);

	// Before fullscreen viewport

	// PASS: OPAQUE GEOMETRY

	unsigned int fsvp = viewportIndexFullscreen;
	const RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];

	{
		// Set clear color
		RenderCommandData::ClearColorData data{ 0.0f, 0.0f, 0.0f, 0.0f };
		commandList.AddControl(fsvp, g_pass, 0, ctrl::ClearColor, sizeof(data), &data);
	}

	{
		// Set viewport size
		RenderCommandData::ViewportData data;
		data.x = 0;
		data.y = 0;
		data.w = gbuffer.width;
		data.h = gbuffer.height;

		commandList.AddControl(fsvp, g_pass, 1, ctrl::Viewport, sizeof(data), &data);
	}

	{
		// Bind geometry framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = RenderFramebufferTarget::Framebuffer;
		data.framebuffer = gbuffer.framebuffer;

		commandList.AddControl(fsvp, g_pass, 2, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	// Clear currently bound RenderFramebufferTarget::Framebuffer
	commandList.AddControl(fsvp, g_pass, 3, ctrl::Clear, colorAndDepthMask);

	// PASS: OPAQUE LIGHTING

	// Draw lighting pass
	commandList.AddDrawWithCallback(fsvp, l_pass, 0.0f, deferredLightingCallback);

	// PASS: SKYBOX

	commandList.AddControl(fsvp, s_pass, 0, ctrl::DepthTestEnable);
	commandList.AddControl(fsvp, s_pass, 1, ctrl::DepthTestFunction, GL_EQUAL);
	commandList.AddControl(fsvp, s_pass, 2, ctrl::DepthWriteDisable);

	// PASS: TRANSPARENT

	// Before transparent objects

	commandList.AddControl(fsvp, t_pass, 0, ctrl::DepthTestFunction, GL_LESS);
	commandList.AddControl(fsvp, t_pass, 1, ctrl::BlendingEnable);

	{
		// Set mix blending
		RenderCommandData::BlendFunctionData data;
		data.srcFactor = RenderBlendFactor::SrcAlpha;
		data.dstFactor = RenderBlendFactor::OneMinusSrcAlpha;

		commandList.AddControl(fsvp, t_pass, 2, ctrl::BlendFunction, sizeof(data), &data);
	}

	// PASS: POST PROCESS

	// Draw HDR tonemapping pass
	commandList.AddDrawWithCallback(fsvp, p_pass, 0.0f, tonemappingCallback);

	// Create draw commands for render objects in scene

	unsigned int visRequired = BitPack::CalculateRequired(data.count);
	objectVisibility.Resize(visRequired * viewportCount);

	const unsigned int compareTrIdx = static_cast<unsigned int>(TransparencyType::AlphaTest);

	BitPack* vis[MaxViewportCount];

	for (size_t vpIdx = 0, count = viewportCount; vpIdx < count; ++vpIdx)
	{
		vis[vpIdx] = objectVisibility.GetData() + visRequired * vpIdx;

		const FrustumPlanes& frustum = viewportData[vpIdx].frustum;
		const Mat4x4f& viewProjection = viewportData[vpIdx].viewProjection;
		const Vec2i viewPortSize = viewportData[vpIdx].viewportRectangle.size;
		float minSize = viewportData[vpIdx].objectMinScreenSizePx / (viewPortSize.x * viewPortSize.y);

		Intersect::FrustumAABBMinSize(frustum, viewProjection, minSize, data.count, data.bounds, vis[vpIdx]);
	}

	unsigned int objectDrawCount = 0;

	for (unsigned int i = 1; i < data.count; ++i)
	{
		Vec3f objPos = (data.transform[i] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

		// Test visibility in shadow viewports
		for (unsigned int vpIdx = 0, count = numShadowViewports; vpIdx < count; ++vpIdx)
		{
			if (BitPack::Get(vis[vpIdx], i) &&
				static_cast<unsigned int>(data.order[i].transparency) <= compareTrIdx)
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
			const RenderOrderData& o = data.order[i];
			const RenderViewport& vp = viewportData[fsvp];

			float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

			RenderPass pass = static_cast<RenderPass>(o.transparency);
			commandList.AddDraw(fsvp, pass, depth, o.material, i);

			objectDrawCount += 1;
		}
	}

	for (unsigned int i = 0, count = customRenderers.GetCount(); i < count; ++i)
	{
		if (customRenderers[i] != nullptr)
		{
			CustomRenderer::CommandParams params;
			params.fullscreenViewport = this->viewportIndexFullscreen;
			params.commandList = &commandList;
			params.callbackId = i + 1;
			params.scene = scene;

			customRenderers[i]->AddRenderCommands(params);
		}
	}

	commandList.Sort();

	return objectDrawCount;
}

void Renderer::ReallocateRenderObjects(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(MeshId) + sizeof(RenderOrderData) +
		sizeof(BoundingBox) + sizeof(Mat4x4f));

	newData.buffer = this->allocator->Allocate(bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.mesh = reinterpret_cast<MeshId*>(newData.entity + required);
	newData.order = reinterpret_cast<RenderOrderData*>(newData.mesh + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.order + required);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.mesh, data.mesh, data.count * sizeof(MeshId));
		std::memcpy(newData.order, data.order, data.count * sizeof(RenderOrderData));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.transform, data.transform, data.count * sizeof(Mat4x4f));

		this->allocator->Deallocate(data.buffer);
	}

	data = newData;
}

RenderObjectId Renderer::AddRenderObject(Entity entity)
{
	RenderObjectId id;
	this->AddRenderObject(1, &entity, &id);
	return id;
}

void Renderer::AddRenderObject(unsigned int count, const Entity* entities, RenderObjectId* renderObjectIdsOut)
{
	if (data.count + count > data.allocated)
		this->ReallocateRenderObjects(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second.i = id;

		data.entity[id] = e;

		renderObjectIdsOut[i].i = id;
	}

	data.count += count;
}

unsigned int Renderer::AddCustomRenderer(CustomRenderer* customRenderer)
{
	for (unsigned int i = 0, count = customRenderers.GetCount(); i < count; ++i)
	{
		if (customRenderers[i] == nullptr)
		{
			customRenderers[i] = customRenderer;

			return i + 1;
		}
	}

	customRenderers.PushBack(customRenderer);
	return customRenderers.GetCount();
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

void Renderer::NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms)
{
	for (unsigned int entityIdx = 0; entityIdx < count; ++entityIdx)
	{
		Entity entity = entities[entityIdx];
		RenderObjectId obj = this->Lookup(entity);

		if (obj.IsNull() == false)
		{
			unsigned int dataIdx = obj.i;

			// Recalculate bounding box
			MeshId meshId = data.mesh[dataIdx];
			BoundingBox* bounds = meshManager->GetBoundingBox(meshId);
			data.bounds[dataIdx] = bounds->Transform(transforms[entityIdx]);

			// Set world transform
			data.transform[dataIdx] = transforms[entityIdx];
		}
	}
}

void Renderer::DebugRender(DebugVectorRenderer* vectorRenderer)
{
	Color color(1.0f, 1.0f, 1.0f, 1.0f);

	for (unsigned int idx = 1; idx < data.count; ++idx)
	{
		Vec3f pos = (data.transform[idx] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
		Vec3f scale = data.bounds[idx].extents * 2.0f;
		Mat4x4f transform = Mat4x4f::Scale(scale) * Mat4x4f::Translate(pos);
		vectorRenderer->DrawWireCube(transform, color);
	}
}
