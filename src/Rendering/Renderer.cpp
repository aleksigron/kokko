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
#include "Rendering/Shader.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MeshPresets.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/Texture.hpp"

#include "Scene/Scene.hpp"

#include "System/IncludeOpenGL.hpp"
#include "System/Window.hpp"

struct RendererFramebuffer
{
	static const unsigned int MaxTextureCount = 4;

	unsigned int framebuffer;
	unsigned int textureCount;
	unsigned int textures[MaxTextureCount];
	int width;
	int height;
	bool used;
};

struct RendererViewport
{
	Vec3f position;
	Vec3f forward;

	float farMinusNear;
	float minusNear;

	Mat4x4f view;
	Mat4x4f projection;
	Mat4x4f viewProjection;

	FrustumPlanes frustum;

	unsigned int framebufferIndex;
};

Renderer::Renderer(Allocator* allocator, RenderDevice* renderDevice, LightManager* lightManager) :
	allocator(allocator),
	device(renderDevice),
	framebufferData(nullptr),
	framebufferCount(0),
	viewportData(nullptr),
	viewportCount(0),
	viewportIndexFullscreen(0),
	entityMap(allocator),
	lightManager(lightManager),
	overrideRenderCamera(nullptr),
	overrideCullingCamera(nullptr),
	commandList(allocator),
	objectVisibility(allocator)
{
	lightingMesh = MeshId{ 0 };
	lightingShader = 0;
	shadowMaterial = MaterialId{ 0 };

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
	{
		// Allocate framebuffer data storage
		void* buf = this->allocator->Allocate(sizeof(RendererFramebuffer) * MaxViewportCount);
		framebufferData = static_cast<RendererFramebuffer*>(buf);
	}

	{
		// Allocate viewport data storage
		void* buf = this->allocator->Allocate(sizeof(RendererViewport) * MaxViewportCount);
		viewportData = static_cast<RendererViewport*>(buf);
	}

	{
		// Create geometry framebuffer and textures

		Vec2f framebufferSize = window->GetFrameBufferSize();

		framebufferCount += 1;

		RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];
		gbuffer.width = static_cast<int>(framebufferSize.x);
		gbuffer.height = static_cast<int>(framebufferSize.y);

		// Create and bind framebuffer

		device->CreateFramebuffers(1, &gbuffer.framebuffer);
		device->BindFramebuffer(GL_FRAMEBUFFER, gbuffer.framebuffer);

		gbuffer.textureCount = 3;
		device->CreateTextures(gbuffer.textureCount, gbuffer.textures);
		unsigned int colAtt[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		// Normal buffer

		unsigned int norTexture = gbuffer.textures[NormalTextureIdx];
		device->BindTexture(GL_TEXTURE_2D, norTexture);

		RenderCommandData::SetTextureImage2D norTextureImage{
			GL_TEXTURE_2D, 0, GL_RG16, gbuffer.width, gbuffer.height, GL_RG, GL_UNSIGNED_SHORT, nullptr
		};
		device->SetTextureImage2D(&norTextureImage);

		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		RenderCommandData::AttachFramebufferTexture2D norAttachTexture{
			GL_FRAMEBUFFER, colAtt[0], GL_TEXTURE_2D, norTexture, 0
		};
		device->AttachFramebufferTexture2D(&norAttachTexture);

		// Albedo color + specular buffer

		unsigned int asTexture = gbuffer.textures[AlbedoSpecTextureIdx];
		device->BindTexture(GL_TEXTURE_2D, asTexture);

		RenderCommandData::SetTextureImage2D asTextureImage{
			GL_TEXTURE_2D, 0, GL_RGBA, gbuffer.width, gbuffer.height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr
		};
		device->SetTextureImage2D(&asTextureImage);

		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		RenderCommandData::AttachFramebufferTexture2D asAttachTexture{
			GL_FRAMEBUFFER, colAtt[1], GL_TEXTURE_2D, asTexture, 0
		};
		device->AttachFramebufferTexture2D(&asAttachTexture);

		// Which color attachments we'll use for rendering
		device->SetFramebufferDrawBuffers(2, colAtt);

		// Create and attach depth buffer
		unsigned int depthTexture = gbuffer.textures[DepthTextureIdx];
		device->BindTexture(GL_TEXTURE_2D, depthTexture);

		RenderCommandData::SetTextureImage2D depthTextureImage{
			GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, gbuffer.width, gbuffer.height, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
		};
		device->SetTextureImage2D(&depthTextureImage);

		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		RenderCommandData::AttachFramebufferTexture2D depthAttachTexture{
			GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0
		};
		device->AttachFramebufferTexture2D(&depthAttachTexture);

		// Reset active texture and framebuffer

		device->BindTexture(GL_TEXTURE_2D, 0);
		device->BindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	Engine* engine = Engine::GetInstance();
	MaterialManager* materialManager = engine->GetMaterialManager();
	MeshManager* meshManager = engine->GetMeshManager();
	ResourceManager* resManager = engine->GetResourceManager();

	{
		// Create screen filling quad
		lightingMesh = meshManager->CreateMesh();
		MeshPresets::UploadPlane(meshManager, lightingMesh);
	}
	
	{
		const char* const matPath = "res/materials/depth.material.json";
		shadowMaterial = materialManager->GetIdByPath(StringRef(matPath));
	}

	{
		static const char* const path = "res/shaders/lighting.shader.json";

		Shader* shader = resManager->GetShader(path);
		lightingShader = shader->nameHash;
	}

	// Create skybox entity
	{
		EntityManager* em = engine->GetEntityManager();
		skyboxEntity = em->Create();

		RenderObjectId skyboxRenderObj = AddRenderObject(skyboxEntity);

		MeshId skyboxMesh = meshManager->CreateMesh();
		MeshPresets::UploadCube(meshManager, skyboxMesh);
		SetMeshId(skyboxRenderObj, skyboxMesh);
	}
}

void Renderer::Deinitialize()
{
	MeshManager* meshManager = Engine::GetInstance()->GetMeshManager();

	if (lightingMesh.IsValid())
		meshManager->RemoveMesh(lightingMesh);

	for (unsigned int i = 0; i < framebufferCount; ++i)
	{
		const RendererFramebuffer& fb = framebufferData[i];

		if (framebufferData[i].framebuffer != 0)
		{
			device->DestroyTextures(framebufferData[i].textureCount, framebufferData[i].textures);
			device->DestroyFramebuffers(1, &framebufferData[i].framebuffer);
		}
	}

	this->allocator->Deallocate(viewportData);
	this->allocator->Deallocate(framebufferData);
}

Camera* Renderer::GetRenderCamera(Scene* scene)
{
	return overrideRenderCamera != nullptr ? overrideRenderCamera : scene->GetActiveCamera();
}

Camera* Renderer::GetCullingCamera(Scene* scene)
{
	return overrideCullingCamera != nullptr ? overrideCullingCamera : scene->GetActiveCamera();
}

void Renderer::Render(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	MeshManager* meshManager = engine->GetMeshManager();
	MaterialManager* materialManager = engine->GetMaterialManager();
	ResourceManager* res = engine->GetResourceManager();

	Camera* renderCamera = this->GetRenderCamera(scene);
	SceneObjectId renderCameraObject = scene->Lookup(renderCamera->GetEntity());
	Mat4x4f renderCameraTransform = scene->GetWorldTransform(renderCameraObject);
	Vec3f cameraPosition = (renderCameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	PopulateCommandList(scene);

	uint64_t* itr = commandList.commands.GetData();
	uint64_t* end = itr + commandList.commands.GetCount();
	for (; itr != end; ++itr)
	{
		uint64_t command = *itr;

		// If command is not control command, draw object
		if (ParseControlCommand(command) == false)
		{
			RenderPass pass = static_cast<RenderPass>(renderOrder.viewportPass.GetValue(command));

			if (pass != RenderPass::OpaqueLighting)
			{
				unsigned int vpIdx = renderOrder.viewportIndex.GetValue(command);
				unsigned int objIdx = renderOrder.renderObject.GetValue(command);
				unsigned int mat = renderOrder.materialId.GetValue(command);
				MaterialId matId = MaterialId{mat};

				const MaterialUniformData& mu = materialManager->GetUniformData(matId);

				unsigned int shaderId = materialManager->GetShaderId(matId);
				Shader* shader = res->GetShader(shaderId);

				device->UseShaderProgram(shader->driverId);

				unsigned int usedTextures = 0;

				// Bind each material uniform with a value
				for (unsigned uIndex = 0; uIndex < mu.count; ++uIndex)
				{
					const MaterialUniform& u = mu.uniforms[uIndex];

					unsigned char* d = mu.data + u.dataOffset;

					switch (u.type)
					{
						case ShaderUniformType::Mat4x4:
							device->SetUniformMat4x4f(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec4:
							device->SetUniformVec4f(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec3:
							device->SetUniformVec3f(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec2:
							device->SetUniformVec2f(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Float:
							device->SetUniformFloat(u.location, *reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Int:
							device->SetUniformInt(u.location, *reinterpret_cast<int*>(d));
							break;

						case ShaderUniformType::Tex2D:
						case ShaderUniformType::TexCube:
						{
							uint32_t textureHash = *reinterpret_cast<uint32_t*>(d);
							Texture* texture = res->GetTexture(textureHash);

							device->SetActiveTextureUnit(usedTextures);
							device->BindTexture(texture->targetType, texture->driverId);
							device->SetUniformInt(u.location, usedTextures);

							++usedTextures;
						}
							break;
					}
				}

				const Mat4x4f& view = viewportData[vpIdx].view;
				const Mat4x4f& projection = viewportData[vpIdx].projection;
				const Mat4x4f& viewProjection = viewportData[vpIdx].viewProjection;
				const Mat4x4f& model = data.transform[objIdx];

				if (shader->uniformMatMVP >= 0)
				{
					Mat4x4f mvp = viewProjection * model;
					device->SetUniformMat4x4f(shader->uniformMatMVP, 1, mvp.ValuePointer());
				}

				if (shader->uniformMatMV >= 0)
				{
					Mat4x4f mv = view * model;
					device->SetUniformMat4x4f(shader->uniformMatMV, 1, mv.ValuePointer());
				}

				if (shader->uniformMatVP >= 0)
				{
					device->SetUniformMat4x4f(shader->uniformMatVP, 1, viewProjection.ValuePointer());
				}

				if (shader->uniformMatM >= 0)
				{
					device->SetUniformMat4x4f(shader->uniformMatM, 1, model.ValuePointer());
				}

				if (shader->uniformMatV >= 0)
				{
					device->SetUniformMat4x4f(shader->uniformMatV, 1, view.ValuePointer());
				}

				if (shader->uniformMatP >= 0)
				{
					device->SetUniformMat4x4f(shader->uniformMatP, 1, projection.ValuePointer());
				}

				MeshId mesh = data.mesh[objIdx];
				MeshDrawData* draw = meshManager->GetDrawData(mesh);
				device->BindVertexArray(draw->vertexArrayObject);
				device->DrawVertexArray(draw->primitiveMode, draw->indexCount, draw->indexElementType);
			}
			else // Pass is OpaqueLighting
			{
				unsigned int objIdx = renderOrder.renderObject.GetValue(command);
				Shader* shader = res->GetShader(lightingShader);

				const RendererViewport& fsvp = viewportData[viewportIndexFullscreen];

				// Update directional light viewports
				Array<LightId> directionalLights(allocator);
				lightManager->GetDirectionalLights(directionalLights);

				Array<LightId> nonDirLights(allocator);
				lightManager->GetNonDirectionalLightsWithinFrustum(fsvp.frustum, nonDirLights);

				Vec2f halfNearPlane;
				halfNearPlane.y = std::tan(renderCamera->parameters.height * 0.5f);
				halfNearPlane.x = halfNearPlane.y * renderCamera->parameters.aspect;

				const unsigned int shaderId = shader->driverId;

				int normLoc = device->GetUniformLocation(shaderId, "g_norm");
				int albSpecLoc = device->GetUniformLocation(shaderId, "g_alb_spec");
				int depthLoc = device->GetUniformLocation(shaderId, "g_depth");

				int halfNearPlaneLoc = device->GetUniformLocation(shaderId, "half_near_plane");
				int persMatLoc = device->GetUniformLocation(shaderId, "pers_mat");

				int cascadeCountLoc = device->GetUniformLocation(shaderId, "shadow_params.cascade_count");
				int nearDepthLoc = device->GetUniformLocation(shaderId, "shadow_params.splits[0]");

				device->UseShaderProgram(shaderId);

				device->SetUniformInt(normLoc, 0);
				device->SetUniformInt(albSpecLoc, 1);
				device->SetUniformInt(depthLoc, 2);

				device->SetUniformVec2f(halfNearPlaneLoc, 1, halfNearPlane.ValuePointer());

				// Set the perspective matrix
				device->SetUniformMat4x4f(persMatLoc, 1, fsvp.projection.ValuePointer());

				// Directional light
				if (directionalLights.GetCount() > 0)
				{
					int lightDirLoc = device->GetUniformLocation(shaderId, "dir_light.direction");
					int lightColLoc = device->GetUniformLocation(shaderId, "dir_light.color");

					LightId dirLightId = directionalLights[0];

					Mat3x3f orientation = lightManager->GetOrientation(dirLightId);
					Vec3f wLightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);
					Vec3f vLightDir = (fsvp.view * Vec4f(wLightDir, 0.0f)).xyz();
					device->SetUniformVec3f(lightDirLoc, 1, vLightDir.ValuePointer());

					Vec3f lightCol = lightManager->GetColor(dirLightId);
					device->SetUniformVec3f(lightColLoc, 1, lightCol.ValuePointer());
				}

				char uniformNameBuf[32];

				// Light other visible lights
				for (unsigned int lightIdx = 0, count = nonDirLights.GetCount(); lightIdx < count; ++lightIdx)
				{
					std::sprintf(uniformNameBuf, "point_light[%u].position", lightIdx);
					int lightPosLoc = device->GetUniformLocation(shaderId, uniformNameBuf);

					std::sprintf(uniformNameBuf, "point_light[%u].color", lightIdx);
					int lightColLoc = device->GetUniformLocation(shaderId, uniformNameBuf);

					LightId pointLightId = nonDirLights[lightIdx];

					Vec3f wLightPos = lightManager->GetPosition(pointLightId);
					Vec3f vLightPos = (fsvp.view * Vec4f(wLightPos, 1.0f)).xyz();
					device->SetUniformVec3f(lightPosLoc, 1, vLightPos.ValuePointer());

					Vec3f lightCol = lightManager->GetColor(pointLightId);
					device->SetUniformVec3f(lightColLoc, 1, lightCol.ValuePointer());
				}

				// Bind textures

				const RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];

				device->SetActiveTextureUnit(0);
				device->BindTexture(GL_TEXTURE_2D, gbuffer.textures[NormalTextureIdx]);
				device->SetActiveTextureUnit(1);
				device->BindTexture(GL_TEXTURE_2D, gbuffer.textures[AlbedoSpecTextureIdx]);
				device->SetActiveTextureUnit(2);
				device->BindTexture(GL_TEXTURE_2D, gbuffer.textures[DepthTextureIdx]);

				unsigned int usedSamplerSlots = 3;

				device->SetUniformFloat(nearDepthLoc, renderCamera->parameters.near);

				unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
				device->SetUniformInt(cascadeCountLoc, shadowCascadeCount);

				// Need for each shadow casting light / shadow cascade
				for (unsigned int vpIdx = 0; vpIdx < viewportCount; ++vpIdx)
				{
					// Format uniform names and find locations

					std::sprintf(uniformNameBuf, "shadow_params.matrices[%u]", vpIdx);
					int shadowMatrixLoc = device->GetUniformLocation(shaderId, uniformNameBuf);

					std::sprintf(uniformNameBuf, "shadow_params.samplers[%u]", vpIdx);
					int shadowSamplerLoc = device->GetUniformLocation(shaderId, uniformNameBuf);

					// shadow_params.splits[0] is the near depth
					std::sprintf(uniformNameBuf, "shadow_params.splits[%u]", vpIdx + 1);
					int cascadeSplitLoc = device->GetUniformLocation(shaderId, uniformNameBuf);

					const RendererViewport& vp = viewportData[vpIdx];

					Mat4x4f bias;
					bias[0] = 0.5; bias[1] = 0.0; bias[2] = 0.0; bias[3] = 0.0;
					bias[4] = 0.0; bias[5] = 0.5; bias[6] = 0.0; bias[7] = 0.0;
					bias[8] = 0.0; bias[9] = 0.0; bias[10] = 0.5; bias[11] = 0.0;
					bias[12] = 0.5; bias[13] = 0.5; bias[14] = 0.5; bias[15] = 1.0;

					float cascadeSplitDepths[CascadedShadowMap::MaxCascadeCount];
					CascadedShadowMap::CalculateSplitDepths(renderCamera->parameters, cascadeSplitDepths);

					Mat4x4f viewToLight = vp.viewProjection * renderCameraTransform;
					Mat4x4f shadowMat = bias * viewToLight;

					device->SetUniformMat4x4f(shadowMatrixLoc, 1, shadowMat.ValuePointer());

					const RendererFramebuffer& fb = framebufferData[vp.framebufferIndex];

					device->SetActiveTextureUnit(usedSamplerSlots + vpIdx);
					device->BindTexture(GL_TEXTURE_2D, fb.textures[0]);

					device->SetUniformInt(shadowSamplerLoc, usedSamplerSlots + vpIdx);

					device->SetUniformFloat(cascadeSplitLoc, cascadeSplitDepths[vpIdx]);
				}

				// Draw fullscreen quad

				MeshDrawData* draw = meshManager->GetDrawData(lightingMesh);
				device->BindVertexArray(draw->vertexArrayObject);
				device->DrawVertexArray(draw->primitiveMode, draw->indexCount, draw->indexElementType);
			}
		}
	}

	commandList.Clear();
	ClearFramebufferUsageFlags();
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
	}

	return true;
}

float CalculateDepth(const Vec3f& objPos, const Vec3f& eyePos, const Vec3f& eyeForward, float farMinusNear, float minusNear)
{
	return (Vec3f::Dot(objPos - eyePos, eyeForward) - minusNear) / farMinusNear;
}

void Renderer::PopulateCommandList(Scene* scene)
{
	// Get camera transforms

	Camera* renderCamera = this->GetRenderCamera(scene);
	const ProjectionParameters& projectionParams = renderCamera->parameters;
	SceneObjectId cameraObject = scene->Lookup(renderCamera->GetEntity());
	Mat4x4f cameraTransform = scene->GetWorldTransform(cameraObject);

	Camera* cullingCamera = this->GetCullingCamera(scene);
	SceneObjectId cullingCameraObject = scene->Lookup(cullingCamera->GetEntity());
	Mat4x4f cullingCameraTransform = scene->GetWorldTransform(cullingCameraObject);

	Vec3f cameraPos = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	Vec3f cameraForward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();

	int shadowSide = CascadedShadowMap::GetShadowCascadeResolution();
	Vec2i shadowSize(shadowSide, shadowSide);
	Mat4x4f cascadeViewTransforms[CascadedShadowMap::MaxCascadeCount];
	ProjectionParameters lightProjections[CascadedShadowMap::MaxCascadeCount];

	unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();

	// Reset the used viewport count
	viewportCount = 0;

	// Update directional light viewports
	Array<LightId> directionalLights(allocator);
	lightManager->GetDirectionalLights(directionalLights);
	
	for (unsigned int i = 0, count = directionalLights.GetCount(); i < count; ++i)
	{
		LightId id = directionalLights[i];

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

				RendererViewport& vp = viewportData[vpIdx];
				vp.position = (cascadeViewTransforms[cascade] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
				vp.forward = (cascadeViewTransforms[cascade] * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
				vp.farMinusNear = lightProjections[cascade].far - lightProjections[cascade].near;
				vp.minusNear = -lightProjections[cascade].near;
				vp.view = cascadeViewTransforms[cascade].GetInverse();
				vp.projection = lightProjections[cascade].GetProjectionMatrix();
				vp.viewProjection = vp.projection * vp.view;
				vp.frustum.Update(lightProjections[cascade], cascadeViewTransforms[cascade]);
				vp.framebufferIndex = GetDepthFramebufferOfSize(shadowSize);
			}
		}
	}

	unsigned int numShadowViewports = viewportCount;

	{
		// Add the fullscreen viewport
		unsigned int vpIdx = viewportCount;
		viewportCount += 1;

		RendererViewport& vp = viewportData[vpIdx];
		vp.position = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
		vp.forward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
		vp.farMinusNear = renderCamera->parameters.far - renderCamera->parameters.near;
		vp.minusNear = -renderCamera->parameters.near;
		vp.view = Camera::GetViewMatrix(cameraTransform);
		vp.projection = projectionParams.GetProjectionMatrix();
		vp.viewProjection = vp.projection * vp.view;
		vp.frustum.Update(cullingCamera->parameters, cullingCameraTransform);
		vp.framebufferIndex = FramebufferIndexGBuffer;

		this->viewportIndexFullscreen = vpIdx;
	}

	const FrustumPlanes& fullscreenFrustum = viewportData[viewportIndexFullscreen].frustum;

	Array<LightId> nonDirLights(allocator);
	lightManager->GetNonDirectionalLightsWithinFrustum(fullscreenFrustum, nonDirLights);

	unsigned int colorAndDepthMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

	RenderPass g_pass = RenderPass::OpaqueGeometry;
	RenderPass l_pass = RenderPass::OpaqueLighting;
	RenderPass s_pass = RenderPass::Skybox;
	RenderPass t_pass = RenderPass::Transparent;

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

	// For each shadow viewport
	for (unsigned int vpIdx = 0; vpIdx < numShadowViewports; ++vpIdx)
	{
		const RendererFramebuffer& framebuffer = framebufferData[viewportData[vpIdx].framebufferIndex];

		{
			// Set viewport size
			RenderCommandData::ViewportData data;
			data.x = 0;
			data.y = 0;
			data.w = framebuffer.width;
			data.h = framebuffer.height;

			commandList.AddControl(vpIdx, g_pass, 7, ctrl::Viewport, sizeof(data), &data);
		}

		{
			// Bind geometry framebuffer
			RenderCommandData::BindFramebufferData data;
			data.target = GL_FRAMEBUFFER;
			data.framebuffer = framebuffer.framebuffer;

			commandList.AddControl(vpIdx, g_pass, 8, ctrl::BindFramebuffer, sizeof(data), &data);
		}

		// Clear currently bound GL_FRAMEBUFFER
		commandList.AddControl(vpIdx, g_pass, 9, ctrl::Clear, GL_DEPTH_BUFFER_BIT);
	}

	// Before fullscreen viewport

	// PASS: OPAQUE GEOMETRY

	unsigned int fsvp = viewportIndexFullscreen;
	const RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];

	{
		// Set clear color
		RenderCommandData::ClearColorData data;
		data.r = 0.0f;
		data.g = 0.0f;
		data.b = 0.0f;
		data.a = 0.0f;

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
		data.target = GL_FRAMEBUFFER;
		data.framebuffer = gbuffer.framebuffer;

		commandList.AddControl(fsvp, g_pass, 2, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	// Clear currently bound GL_FRAMEBUFFER
	commandList.AddControl(fsvp, g_pass, 3, ctrl::Clear, colorAndDepthMask);

	// PASS: OPAQUE LIGHTING

	{
		// Bind default framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = GL_FRAMEBUFFER;
		data.framebuffer = 0;

		commandList.AddControl(fsvp, l_pass, 0, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	commandList.AddControl(fsvp, l_pass, 1, ctrl::Clear, colorAndDepthMask);

	// Set depth test to always pass
	commandList.AddControl(fsvp, l_pass, 2, ctrl::DepthTestFunction, GL_ALWAYS);

	// Draw lighting pass
	commandList.AddDraw(fsvp, l_pass, 0.0f, MaterialId{}, 0);

	// PASS: SKYBOX

	commandList.AddControl(fsvp, s_pass, 0, ctrl::DepthTestFunction, GL_EQUAL);
	commandList.AddControl(fsvp, s_pass, 1, ctrl::DepthWriteDisable);

	// PASS: TRANSPARENT

	// Before transparent objects

	commandList.AddControl(fsvp, t_pass, 0, ctrl::DepthTestFunction, GL_LESS);
	commandList.AddControl(fsvp, t_pass, 1, ctrl::BlendingEnable);

	{
		// Set mix blending
		RenderCommandData::BlendFunctionData data;
		data.srcFactor = GL_SRC_ALPHA;
		data.dstFactor = GL_ONE_MINUS_SRC_ALPHA;

		commandList.AddControl(fsvp, t_pass, 2, ctrl::BlendFunction, sizeof(data), &data);
	}

	// Create draw commands for render objects in scene

	FrustumPlanes frustum[MaxViewportCount];
	frustum[fsvp].Update(cullingCamera->parameters, cullingCameraTransform);

	unsigned int visRequired = BitPack::CalculateRequired(data.count);
	objectVisibility.Resize(visRequired * viewportCount);

	const unsigned int compareTrIdx = static_cast<unsigned int>(TransparencyType::AlphaTest);

	BitPack* vis[MaxViewportCount];

	for (unsigned int vpIdx = 0, count = viewportCount; vpIdx < count; ++vpIdx)
	{
		vis[vpIdx] = objectVisibility.GetData() + visRequired * vpIdx;
		Intersect::FrustumAABB(viewportData[vpIdx].frustum, data.count, data.bounds, vis[vpIdx]);
	}

	for (unsigned int i = 1; i < data.count; ++i)
	{
		Vec3f objPos = (data.transform[i] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

		// Test visibility in shadow viewports
		for (unsigned int vpIdx = 0, count = numShadowViewports; vpIdx < count; ++vpIdx)
		{
			if (BitPack::Get(vis[vpIdx], i) &&
				static_cast<unsigned int>(data.order[i].transparency) <= compareTrIdx)
			{
				const RendererViewport& vp = viewportData[vpIdx];

				float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

				commandList.AddDraw(vpIdx, RenderPass::OpaqueGeometry, depth, shadowMaterial, i);
			}
		}

		// Test visibility in fullscreen viewport
		if (BitPack::Get(vis[fsvp], i))
		{
			const RenderOrderData& o = data.order[i];
			const RendererViewport& vp = viewportData[fsvp];

			float depth = CalculateDepth(objPos, vp.position, vp.forward, vp.farMinusNear, vp.minusNear);

			RenderPass pass = static_cast<RenderPass>(o.transparency);
			commandList.AddDraw(fsvp, pass, depth, o.material, i);
		}
	}

	// Render skybox
	{
		RenderObjectId skyboxRenderObj = Lookup(skyboxEntity);
		MaterialId skyboxMaterial = scene->GetSkyboxMaterial();
		commandList.AddDraw(fsvp, RenderPass::Skybox, 0.0f, skyboxMaterial, skyboxRenderObj.i);

		Mat4x4f skyboxTransform = Mat4x4f::Translate(cameraPos);
		data.transform[skyboxRenderObj.i] = skyboxTransform;
	}

	commandList.Sort();
}

unsigned int Renderer::GetDepthFramebufferOfSize(const Vec2i& size)
{
	// Try to find existing unused framebuffer that matches the requested size
	// Skip index 0 as it is the G-Buffer
	for (unsigned int i = 1; i < framebufferCount; ++i)
	{
		RendererFramebuffer& fb = framebufferData[i];
		if (fb.used == false && fb.width == size.x && fb.height == size.y)
		{
			fb.used = true;
			return i;
		}
	}

	// If no existing framebuffer was found, create a new one if space permits
	if (framebufferCount < MaxViewportCount)
	{
		unsigned int fbIdx = framebufferCount;
		framebufferCount += 1;

		RendererFramebuffer& framebuffer = framebufferData[fbIdx];

		framebuffer.width = size.x;
		framebuffer.height = size.y;
		framebuffer.used = true;

		// Create and bind framebuffer

		device->CreateFramebuffers(1, &framebuffer.framebuffer);
		device->BindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		// We aren't rendering to any color attachments
		unsigned int drawBuffers = GL_NONE;
		device->SetFramebufferDrawBuffers(1, &drawBuffers);

		// Create texture
		framebuffer.textureCount = 1;
		device->CreateTextures(framebuffer.textureCount, &framebuffer.textures[0]);

		// Create and attach depth buffer
		unsigned int depthTexture = framebuffer.textures[0];
		device->BindTexture(GL_TEXTURE_2D, depthTexture);

		RenderCommandData::SetTextureImage2D depthTextureImage{
			GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr
		};
		device->SetTextureImage2D(&depthTextureImage);

		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		device->SetTextureParameterInt(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

		RenderCommandData::AttachFramebufferTexture2D depthFramebufferTexture{
			GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0
		};
		device->AttachFramebufferTexture2D(&depthFramebufferTexture);

		// Clear texture and framebuffer binds
		device->BindTexture(GL_TEXTURE_2D, 0);
		device->BindFramebuffer(GL_FRAMEBUFFER, 0);

		return fbIdx;
	}

	return 0;
}

void Renderer::ClearFramebufferUsageFlags()
{
	// Skip index 0 as it is the G-Buffer
	for (unsigned int i = 1; i < framebufferCount; ++i)
	{
		framebufferData[i].used = false;
	}
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

void Renderer::NotifyUpdatedTransforms(unsigned int count, const Entity* entities, const Mat4x4f* transforms)
{
	MeshManager* meshManager = Engine::GetInstance()->GetMeshManager();

	for (unsigned int entityIdx = 0; entityIdx < count; ++entityIdx)
	{
		Entity entity = entities[entityIdx];
		RenderObjectId obj = this->Lookup(entity);

		if (obj.IsNull() == false)
		{
			unsigned int dataIdx = obj.i;

			// Recalculate bounding box
			BoundingBox* bounds = meshManager->GetBoundingBox(data.mesh[dataIdx]);
			data.bounds[dataIdx] = bounds->Transform(transforms[entityIdx]);

			// Set world transform
			data.transform[dataIdx] = transforms[entityIdx];
		}
	}
}
