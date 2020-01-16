#include "Renderer.hpp"

#include <cstring>
#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine.hpp"
#include "Window.hpp"
#include "Memory/Allocator.hpp"

#include "ResourceManager.hpp"
#include "MaterialManager.hpp"
#include "LightManager.hpp"
#include "MeshManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Scene.hpp"

#include "Camera.hpp"
#include "Math/Rectangle.hpp"
#include "Math/BoundingBox.hpp"
#include "Intersect3D.hpp"
#include "BitPack.hpp"
#include "CascadedShadowMap.hpp"

#include "RenderPipeline.hpp"
#include "RenderCommandData.hpp"
#include "RenderCommandType.hpp"

#include "Sort.hpp"

Renderer::Renderer(Allocator* allocator, LightManager* lightManager) :
	allocator(allocator),
	framebufferData(nullptr),
	framebufferCount(0),
	viewportData(nullptr),
	viewportCount(0),
	viewportIndexFullscreen(0),
	entityMap(allocator),
	lightManager(lightManager),
	overrideRenderCamera(nullptr),
	overrideCullingCamera(nullptr)
{
	lightingData = LightingData{};

	data = InstanceData{};
	data.count = 1; // Reserve index 0 as RenderObjectId::Null value

	this->ReallocateRenderObjects(512);
}

Renderer::~Renderer()
{
	this->Deinitialize();

	this->allocator->Deallocate(data.buffer);
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

		Vec2f sf = window->GetFrameBufferSize();
		Vec2i s(static_cast<float>(sf.x), static_cast<float>(sf.y));

		framebufferCount += 1;

		RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];
		gbuffer.resolution = s;

		// Create and bind framebuffer

		glGenFramebuffers(1, &gbuffer.framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.framebuffer);

		gbuffer.textureCount = 3;
		glGenTextures(gbuffer.textureCount, gbuffer.textures);
		unsigned int colAtt[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

		// Normal buffer
		unsigned int norTexture = gbuffer.textures[NormalTextureIdx];
		glBindTexture(GL_TEXTURE_2D, norTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, s.x, s.y, 0, GL_RGB, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, colAtt[0], GL_TEXTURE_2D, norTexture, 0);

		// Albedo color + specular buffer
		unsigned int asTexture = gbuffer.textures[AlbedoSpecTextureIdx];
		glBindTexture(GL_TEXTURE_2D, asTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s.x, s.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, colAtt[1], GL_TEXTURE_2D, asTexture, 0);

		// Which color attachments we'll use for rendering
		glDrawBuffers(2, colAtt);

		// Create and attach depth buffer
		unsigned int depthTexture = gbuffer.textures[DepthTextureIdx];
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, s.x, s.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

		glBindTexture(GL_TEXTURE_2D, 0);

		// Finally check if framebuffer is complete
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	{
		// Create screen filling quad

		static const Vertex3f vertexData[] = {
			Vertex3f{ Vec3f(-1.0f, -1.0f, 0.0f) },
			Vertex3f{ Vec3f(1.0f, -1.0f, 0.0f) },
			Vertex3f{ Vec3f(-1.0f, 1.0f, 0.0f) },
			Vertex3f{ Vec3f(1.0f, 1.0f, 0.0f) }
		};

		static const unsigned short indexData[] = { 0, 1, 2, 1, 3, 2 };

		MeshManager* meshManager = Engine::GetInstance()->GetMeshManager();

		lightingData.dirMesh = meshManager->CreateMesh();

		IndexedVertexData<Vertex3f, unsigned short> data;
		data.primitiveMode = MeshPrimitiveMode::Triangles;
		data.vertData = vertexData;
		data.vertCount = sizeof(vertexData) / sizeof(Vertex3f);
		data.idxData = indexData;
		data.idxCount = sizeof(indexData) / sizeof(unsigned short);

		meshManager->Upload_3f(lightingData.dirMesh, data);

		MaterialManager* materialManager = Engine::GetInstance()->GetMaterialManager();
		const char* const matPath = "res/materials/depth.material.json";
		lightingData.shadowMaterial = materialManager->GetIdByPath(StringRef(matPath));
	}

	{
		ResourceManager* resManager = Engine::GetInstance()->GetResourceManager();

		static const char* const path = "res/shaders/lighting.shader.json";

		Shader* shader = resManager->GetShader(path);
		lightingData.dirShaderHash = shader->nameHash;
	}
}

void Renderer::Deinitialize()
{
	MeshManager* meshManager = Engine::GetInstance()->GetMeshManager();

	if (lightingData.dirMesh.IsValid())
		meshManager->RemoveMesh(lightingData.dirMesh);

	for (unsigned int i = 0; i < framebufferCount; ++i)
	{
		const RendererFramebuffer& fb = framebufferData[i];

		if (framebufferData[i].framebuffer != 0)
		{
			glDeleteTextures(framebufferData[i].textureCount, framebufferData[i].textures);
			glDeleteFramebuffers(1, &framebufferData[i].framebuffer);
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

	if (scene->skybox.IsInitialized()) // Update skybox transform
		scene->skybox.UpdateTransform(cameraPosition);

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

				glUseProgram(shader->driverId);

				unsigned int usedTextures = 0;

				// Bind each material uniform with a value
				for (unsigned uIndex = 0; uIndex < mu.count; ++uIndex)
				{
					const MaterialUniform& u = mu.uniforms[uIndex];

					unsigned char* d = mu.data + u.dataOffset;

					switch (u.type)
					{
						case ShaderUniformType::Mat4x4:
							glUniformMatrix4fv(u.location, 1, GL_FALSE, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec4:
							glUniform4fv(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec3:
							glUniform3fv(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Vec2:
							glUniform2fv(u.location, 1, reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Float:
							glUniform1f(u.location, *reinterpret_cast<float*>(d));
							break;

						case ShaderUniformType::Int:
							glUniform1i(u.location, *reinterpret_cast<int*>(d));
							break;

						case ShaderUniformType::Tex2D:
						case ShaderUniformType::TexCube:
						{
							uint32_t textureHash = *reinterpret_cast<uint32_t*>(d);
							Texture* texture = res->GetTexture(textureHash);

							glActiveTexture(GL_TEXTURE0 + usedTextures);
							glBindTexture(texture->targetType, texture->driverId);
							glUniform1i(u.location, usedTextures);

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
					glUniformMatrix4fv(shader->uniformMatMVP, 1, GL_FALSE, mvp.ValuePointer());
				}

				if (shader->uniformMatMV >= 0)
				{
					Mat4x4f mv = view * model;
					glUniformMatrix4fv(shader->uniformMatMV, 1, GL_FALSE, mv.ValuePointer());
				}

				if (shader->uniformMatVP >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatVP, 1, GL_FALSE, viewProjection.ValuePointer());
				}

				if (shader->uniformMatM >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatM, 1, GL_FALSE, model.ValuePointer());
				}

				if (shader->uniformMatV >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatV, 1, GL_FALSE, view.ValuePointer());
				}

				if (shader->uniformMatP >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatP, 1, GL_FALSE, projection.ValuePointer());
				}

				MeshId mesh = data.mesh[objIdx];
				MeshDrawData* draw = meshManager->GetDrawData(mesh);
				
				glBindVertexArray(draw->vertexArrayObject);

				glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);
			}
			else // Pass is OpaqueLighting
			{
				ResourceManager* resManager = Engine::GetInstance()->GetResourceManager();
				Shader* shader = resManager->GetShader(lightingData.dirShaderHash);

				const RendererViewport& fsvp = viewportData[viewportIndexFullscreen];

				Vec2f halfNearPlane;
				halfNearPlane.y = std::tan(renderCamera->parameters.height * 0.5f);
				halfNearPlane.x = halfNearPlane.y * renderCamera->parameters.aspect;

				const unsigned int shaderId = shader->driverId;

				int normLoc = glGetUniformLocation(shaderId, "g_norm");
				int albSpecLoc = glGetUniformLocation(shaderId, "g_alb_spec");
				int depthLoc = glGetUniformLocation(shaderId, "g_depth");

				int halfNearPlaneLoc = glGetUniformLocation(shaderId, "half_near_plane");
				int persMatLoc = glGetUniformLocation(shaderId, "pers_mat");

				int lightDirLoc = glGetUniformLocation(shaderId, "light.direction");
				int lightColLoc = glGetUniformLocation(shaderId, "light.color");

				int cascadeCountLoc = glGetUniformLocation(shaderId, "shadow_params.cascade_count");
				int nearDepthLoc = glGetUniformLocation(shaderId, "shadow_params.splits[0]");

				glUseProgram(shaderId);

				glUniform1i(normLoc, 0);
				glUniform1i(albSpecLoc, 1);
				glUniform1i(depthLoc, 2);

				glUniform2f(halfNearPlaneLoc, halfNearPlane.x, halfNearPlane.y);

				// Set the perspective matrix
				glUniformMatrix4fv(persMatLoc, 1, GL_FALSE, fsvp.projection.ValuePointer());

				Vec3f wLightDir = primaryDirectionalLightDirection;
				Vec3f vLightDir = (fsvp.view * Vec4f(wLightDir, 0.0f)).xyz();

				// Set light properties
				glUniform3f(lightDirLoc, vLightDir.x, vLightDir.y, vLightDir.z);

				// TODO: Get light color
				glUniform3f(lightColLoc, 1.0f, 1.0f, 1.0f);

				Mat4x4f bias;
				bias[0] = 0.5; bias[1] = 0.0; bias[2] = 0.0; bias[3] = 0.0;
				bias[4] = 0.0; bias[5] = 0.5; bias[6] = 0.0; bias[7] = 0.0;
				bias[8] = 0.0; bias[9] = 0.0; bias[10] = 0.5; bias[11] = 0.0;
				bias[12] = 0.5; bias[13] = 0.5; bias[14] = 0.5; bias[15] = 1.0;

				float cascadeSplitDepths[CascadedShadowMap::MaxCascadeCount];
				CascadedShadowMap::CalculateSplitDepths(renderCamera->parameters, cascadeSplitDepths);
				 
				// Bind textures

				const RendererFramebuffer& gbuffer = framebufferData[FramebufferIndexGBuffer];

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gbuffer.textures[NormalTextureIdx]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gbuffer.textures[AlbedoSpecTextureIdx]);
				glActiveTexture(GL_TEXTURE2);
				glBindTexture(GL_TEXTURE_2D, gbuffer.textures[DepthTextureIdx]);

				unsigned int usedSamplerSlots = 3;

				glUniform1f(nearDepthLoc, renderCamera->parameters.near);

				unsigned int shadowCascadeCount = CascadedShadowMap::GetCascadeCount();
				glUniform1i(cascadeCountLoc, shadowCascadeCount);

				char uniformNameBuf[32];

				// Need for each shadow casting light / shadow cascade
				for (unsigned int vpIdx = 0; vpIdx < viewportCount; ++vpIdx)
				{
					// Format uniform names and find locations

					std::sprintf(uniformNameBuf, "shadow_params.matrices[%d]", vpIdx);
					int shadowMatrixLoc = glGetUniformLocation(shaderId, uniformNameBuf);

					std::sprintf(uniformNameBuf, "shadow_params.samplers[%d]", vpIdx);
					int shadowSamplerLoc = glGetUniformLocation(shaderId, uniformNameBuf);

					// shadow_params.splits[0] is the near depth
					std::sprintf(uniformNameBuf, "shadow_params.splits[%d]", vpIdx + 1);
					int cascadeSplitLoc = glGetUniformLocation(shaderId, uniformNameBuf);

					const RendererViewport& vp = viewportData[vpIdx];

					Mat4x4f viewToLight = vp.viewProjection * renderCameraTransform;
					Mat4x4f shadowMat = bias * viewToLight;

					glUniformMatrix4fv(shadowMatrixLoc, 1, GL_FALSE, shadowMat.ValuePointer());

					const RendererFramebuffer& fb = framebufferData[vp.framebufferIndex];

					glActiveTexture(GL_TEXTURE0 + usedSamplerSlots + vpIdx);
					glBindTexture(GL_TEXTURE_2D, fb.textures[0]);

					glUniform1i(shadowSamplerLoc, usedSamplerSlots + vpIdx);

					glUniform1f(cascadeSplitLoc, cascadeSplitDepths[vpIdx]);
				}

				// Draw fullscreen quad

				MeshDrawData* draw = meshManager->GetDrawData(lightingData.dirMesh);
				glBindVertexArray(draw->vertexArrayObject);

				glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);
			}
		}
	}

	glBindVertexArray(0);

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
			RenderPipeline::BlendingEnable();
			break;

		case RenderControlType::BlendingDisable:
			RenderPipeline::BlendingDisable();
			break;

		case RenderControlType::Viewport:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* viewport = reinterpret_cast<RenderCommandData::ViewportData*>(data);
			RenderPipeline::Viewport(viewport);
		}
			break;

		case RenderControlType::DepthRange:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* depthRange = reinterpret_cast<RenderCommandData::DepthRangeData*>(data);
			RenderPipeline::DepthRange(depthRange);
		}
			break;

		case RenderControlType::DepthTestEnable:
			RenderPipeline::DepthTestEnable();
			break;

		case RenderControlType::DepthTestDisable:
			RenderPipeline::DepthTestDisable();
			break;

		case RenderControlType::DepthTestFunction:
		{
			unsigned int fn = renderOrder.commandData.GetValue(orderKey);
			RenderPipeline::DepthTestFunction(fn);
		}
			break;

		case RenderControlType::DepthWriteEnable:
			RenderPipeline::DepthWriteEnable();
			break;

		case RenderControlType::DepthWriteDisable:
			RenderPipeline::DepthWriteDisable();
			break;

		case RenderControlType::CullFaceEnable:
			RenderPipeline::CullFaceEnable();
			break;

		case RenderControlType::CullFaceDisable:
			RenderPipeline::CullFaceDisable();
			break;

		case RenderControlType::CullFaceFront:
			RenderPipeline::CullFaceFront();
			break;

		case RenderControlType::CullFaceBack:
			RenderPipeline::CullFaceBack();
			break;

		case RenderControlType::Clear:
		{
			unsigned int mask = renderOrder.commandData.GetValue(orderKey);
			RenderPipeline::Clear(mask);
		}
			break;

		case RenderControlType::ClearColor:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* color = reinterpret_cast<RenderCommandData::ClearColorData*>(data);
			RenderPipeline::ClearColor(color);
		}
			break;

		case RenderControlType::ClearDepth:
		{
			unsigned int intDepth = renderOrder.commandData.GetValue(orderKey);
			float depth = *reinterpret_cast<float*>(&intDepth);
			RenderPipeline::ClearDepth(depth);
		}
			break;

		case RenderControlType::BindFramebuffer:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* bind = reinterpret_cast<RenderCommandData::BindFramebufferData*>(data);
			RenderPipeline::BindFramebuffer(bind);
		}
			break;
			
		case RenderControlType::BlitFramebuffer:
		{
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* blit = reinterpret_cast<RenderCommandData::BlitFramebufferData*>(data);
			RenderPipeline::BlitFramebuffer(blit);
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
	LightId directionalLights = lightManager->GetDirectionalLights();
	// directionalLights somehow gets its data corrupted when returning to this function TODO CONTINUE HERE XXXXXXXXXXXXXXXXXXXXXXX
	for (unsigned int i = 0, count = 1; i < count; ++i)
	{
		LightId id = directionalLights;
		if (lightManager->GetShadowCasting(id) &&
			viewportCount + shadowCascadeCount + 1 <= MaxViewportCount)
		{
			Mat3x3f orientation = lightManager->GetOrientation(id);
			Vec3f lightDir = orientation * Vec3f(0.0f, 0.0f, -1.0f);

			primaryDirectionalLightDirection = lightDir;

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
			data.w = framebuffer.resolution.x;
			data.h = framebuffer.resolution.y;

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
		data.w = gbuffer.resolution.x;
		data.h = gbuffer.resolution.y;

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

	// Clear currently bound GL_FRAMEBUFFER
	commandList.AddControl(fsvp, l_pass, 1, ctrl::Clear, colorAndDepthMask);

	commandList.AddControl(fsvp, l_pass, 2, ctrl::DepthTestFunction, GL_ALWAYS);

	// Do lighting
	commandList.AddDraw(fsvp, l_pass, 0.0f, MaterialId{}, 0);

	// PASS: SKYBOX

	commandList.AddControl(fsvp, s_pass, 0, ctrl::DepthTestFunction, GL_EQUAL);

	// Disable depth writing
	commandList.AddControl(fsvp, s_pass, 1, ctrl::DepthWriteDisable);

	// PASS: TRANSPARENT

	// Before transparent objects

	commandList.AddControl(fsvp, t_pass, 0, ctrl::DepthTestFunction, GL_LESS);

	// Enable blending
	commandList.AddControl(fsvp, t_pass, 1, ctrl::BlendingEnable);

	// Create draw commands for render objects in scene

	MaterialId shadowMaterial = lightingData.shadowMaterial;

	FrustumPlanes frustum[MaxViewportCount];
	frustum[fsvp].Update(cullingCamera->parameters, cullingCameraTransform);

	unsigned int visRequired = BitPack::CalculateRequired(data.count);
	objectVisibility.Resize(visRequired * viewportCount);

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
			if (BitPack::Get(vis[vpIdx], i))
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

	commandList.Sort();
}

unsigned int Renderer::GetDepthFramebufferOfSize(const Vec2i& size)
{
	// Try to find existing unused framebuffer that matches the requested size
	// Skip index 0 as it is the G-Buffer
	for (unsigned int i = 1; i < framebufferCount; ++i)
	{
		RendererFramebuffer& fb = framebufferData[i];
		if (fb.used == false && fb.resolution.x == size.x && fb.resolution.y == size.y)
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

		framebuffer.resolution = size;
		framebuffer.used = true;

		// Create and bind framebuffer

		glGenFramebuffers(1, &framebuffer.framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.framebuffer);

		// We aren't rendering to any color attachments
		glDrawBuffer(GL_NONE);

		// Create texture
		framebuffer.textureCount = 1;
		glGenTextures(framebuffer.textureCount, &framebuffer.textures[0]);

		// Create and attach depth buffer
		unsigned int depthTexture = framebuffer.textures[0];
		glBindTexture(GL_TEXTURE_2D, depthTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);

		// Clear texture bind
		glBindTexture(GL_TEXTURE_2D, 0);

		// Clear framebuffer bind
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
		mapPair->value.i = id;

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
