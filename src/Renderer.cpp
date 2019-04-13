#include "Renderer.hpp"

#include <cstring>
#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Engine.hpp"
#include "App.hpp"
#include "Window.hpp"

#include "ResourceManager.hpp"
#include "MaterialManager.hpp"
#include "MeshManager.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Scene.hpp"

#include "Camera.hpp"
#include "Rectangle.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "FrustumCulling.hpp"
#include "BitPack.hpp"

#include "RenderPipeline.hpp"
#include "RenderCommandData.hpp"
#include "RenderCommandType.hpp"

#include "Sort.hpp"

Renderer::Renderer() :
	overrideRenderCamera(nullptr),
	overrideCullingCamera(nullptr)
{
	lightingData = LightingData{};
	gbuffer = GBufferData{};
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as RenderObjectId::Null value

	this->Reallocate(512);
}

Renderer::~Renderer()
{
	this->Deinitialize();

	operator delete[](data.buffer);
}

void Renderer::Initialize(Window* window)
{
	Vec2f sf = window->GetFrameBufferSize();
	Vec2i s(static_cast<float>(sf.x), static_cast<float>(sf.y));
	gbuffer.framebufferSize = s;

	// Create and bind framebuffer

	glGenFramebuffers(1, &gbuffer.framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.framebuffer);

	glGenTextures(2, gbuffer.textures);
	unsigned int colAtt[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	// Normal buffer
	unsigned int norTexture = gbuffer.textures[GBufferData::Normal];
	glBindTexture(GL_TEXTURE_2D, norTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, s.x, s.y, 0, GL_RGB, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, colAtt[0], GL_TEXTURE_2D, norTexture, 0);

	// Albedo color + specular buffer
	unsigned int asTexture = gbuffer.textures[GBufferData::AlbedoSpec];
	glBindTexture(GL_TEXTURE_2D, asTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, s.x, s.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, colAtt[1], GL_TEXTURE_2D, asTexture, 0);

	// Which color attachments we'll use for rendering
	glDrawBuffers(2, colAtt);

	// Create and attach depth buffer

	glGenRenderbuffers(1, &gbuffer.depthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, gbuffer.depthRenderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, s.x, s.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuffer.depthRenderbuffer);

	// Finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

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

	if (gbuffer.framebuffer != 0)
	{
		glDeleteTextures(3, gbuffer.textures);
		glDeleteRenderbuffers(1, &gbuffer.depthRenderbuffer);
		glDeleteFramebuffers(1, &gbuffer.framebuffer);
	}
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

	Camera* cullingCamera = this->GetCullingCamera(scene);

	SceneObjectId cullingCameraObject = scene->Lookup(cullingCamera->GetEntity());
	Mat4x4f cullingCameraTransform = scene->GetWorldTransform(cullingCameraObject);

	ViewFrustum frustum;
	frustum.UpdateFrustum(*cullingCamera, cullingCameraTransform);

	// Retrieve updated transforms
	scene->NotifyUpdatedTransforms(this);

	// Do frustum culling
	FrustumCulling::CullAABB(frustum, data.count, data.bounds, data.visibility);

	CreateDrawCalls(scene);

	Mat4x4f viewMatrix = Camera::GetViewMatrix(renderCameraTransform);
	Mat4x4f projMatrix = renderCamera->GetProjectionMatrix();
	Mat4x4f viewProjection = projMatrix * viewMatrix;

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

				const Mat4x4f& modelMatrix = data.transform[objIdx];

				if (shader->uniformMatMVP >= 0)
				{
					Mat4x4f mvp = viewProjection * modelMatrix;
					glUniformMatrix4fv(shader->uniformMatMVP, 1, GL_FALSE, mvp.ValuePointer());
				}

				if (shader->uniformMatMV >= 0)
				{
					Mat4x4f mv = viewMatrix * modelMatrix;
					glUniformMatrix4fv(shader->uniformMatMV, 1, GL_FALSE, mv.ValuePointer());
				}

				if (shader->uniformMatVP >= 0)
				{
					Mat4x4f vp = projMatrix * viewMatrix;
					glUniformMatrix4fv(shader->uniformMatVP, 1, GL_FALSE, vp.ValuePointer());
				}

				if (shader->uniformMatM >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatM, 1, GL_FALSE, modelMatrix.ValuePointer());
				}

				if (shader->uniformMatV >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatV, 1, GL_FALSE, viewMatrix.ValuePointer());
				}

				if (shader->uniformMatP >= 0)
				{
					glUniformMatrix4fv(shader->uniformMatP, 1, GL_FALSE, projMatrix.ValuePointer());
				}

				MeshDrawData* draw = meshManager->GetDrawData(data.mesh[objIdx]);
				glBindVertexArray(draw->vertexArrayObject);

				glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);
			}
			else // Pass is OpaqueLighting
			{
				ResourceManager* resManager = Engine::GetInstance()->GetResourceManager();
				Shader* shader = resManager->GetShader(lightingData.dirShaderHash);

				const unsigned int shaderId = shader->driverId;

				unsigned int normLoc = glGetUniformLocation(shaderId, "g_norm");
				unsigned int albSpecLoc = glGetUniformLocation(shaderId, "g_alb_spec");
				unsigned int invLightDirLoc = glGetUniformLocation(shaderId, "invLightDir");
				unsigned int lightColLoc = glGetUniformLocation(shaderId, "lightCol");

				glUseProgram(shaderId);

				glUniform1i(normLoc, 0);
				glUniform1i(albSpecLoc, 1);

				Vec3f wInvLightDir = Vec3f(0.5f, 1.5f, 0.8f).GetNormalized();
				Vec3f lightDir = (viewMatrix * Vec4f(wInvLightDir, 0.0f)).xyz();

				// Set view-space inverse light direction
				glUniform3f(invLightDirLoc, lightDir.x, lightDir.y, lightDir.z);
				glUniform3f(lightColLoc, 1.0f, 1.0f, 1.0f); // Set light color

				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, gbuffer.textures[GBufferData::Normal]);
				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, gbuffer.textures[GBufferData::AlbedoSpec]);

				MeshDrawData* draw = meshManager->GetDrawData(lightingData.dirMesh);
				glBindVertexArray(draw->vertexArrayObject);

				glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);
			}
		}
	}

	glBindVertexArray(0);

	commandList.Clear();
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
			unsigned int offset = renderOrder.commandData.GetValue(orderKey);
			uint8_t* data = commandList.commandData.GetData() + offset;
			auto* clear = reinterpret_cast<RenderCommandData::ClearData*>(data);
			RenderPipeline::Clear(clear);
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

void Renderer::CreateDrawCalls(Scene* scene)
{
	using ctrl = RenderControlType;

	Vec2i s = gbuffer.framebufferSize;

	Camera* renderCamera = this->GetRenderCamera(scene);

	SceneObjectId cameraObject = scene->Lookup(renderCamera->GetEntity());
	Mat4x4f cameraTransform = scene->GetWorldTransform(cameraObject);

	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	Vec3f cameraForward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
	
	float farPlane = renderCamera->farClipDistance;

	RenderCommandSlot s_o_slot(SceneLayer::Skybox, RenderPass::OpaqueGeometry);
	RenderCommandSlot s_t_slot(SceneLayer::Skybox, RenderPass::Transparent);
	RenderCommandSlot w_o_slot(SceneLayer::World, RenderPass::OpaqueGeometry);
	RenderCommandSlot w_l_slot(SceneLayer::World, RenderPass::OpaqueLighting);
	RenderCommandSlot w_t_slot(SceneLayer::World, RenderPass::Transparent);

	// Before everything

	// Enable depth writing
	commandList.AddControl(s_o_slot, 0, ctrl::DepthWriteEnable);

	// Set depth test on
	commandList.AddControl(s_o_slot, 1, ctrl::DepthTestEnable);

	// Set depth test function
	commandList.AddControl(s_o_slot, 2, ctrl::DepthTestFunction, GL_LESS);

	// Set face culling on
	commandList.AddControl(s_o_slot, 3, ctrl::CullFaceEnable);

	// Set face culling to cull back faces
	commandList.AddControl(s_o_slot, 4, ctrl::CullFaceBack);

	// Before opaque objects in skybox layer

	{
		// Bind default framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = GL_FRAMEBUFFER;
		data.framebuffer = 0;

		commandList.AddControl(s_o_slot, 5, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	// Disable depth writing
	commandList.AddControl(s_o_slot, 6, ctrl::DepthWriteDisable);

	// Disable blending
	commandList.AddControl(s_o_slot, 7, ctrl::BlendingDisable);

	// Before transparent objects in skybox layer

	// Enable blending
	commandList.AddControl(s_t_slot, 0, ctrl::BlendingEnable);

	// PASS: OPAQUE GEOMETRY

	// Enable depth writing
	commandList.AddControl(w_o_slot, 0, ctrl::DepthWriteEnable);

	// Disable blending
	commandList.AddControl(w_o_slot, 1, ctrl::BlendingDisable);

	{
		// Bind geometry framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = GL_FRAMEBUFFER;
		data.framebuffer = gbuffer.framebuffer;

		commandList.AddControl(w_o_slot, 2, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	{
		// Clear currently bound GL_FRAMEBUFFER
		RenderCommandData::ClearData data;
		data.r = 0.0f;
		data.g = 0.0f;
		data.b = 0.0f;
		data.a = 1.0f;
		data.mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

		commandList.AddControl(w_o_slot, 3, ctrl::Clear, sizeof(data), &data);
	}

	// PASS: OPAQUE LIGHTING

	{
		// Bind default framebuffer
		RenderCommandData::BindFramebufferData data;
		data.target = GL_FRAMEBUFFER;
		data.framebuffer = 0;

		commandList.AddControl(w_l_slot, 0, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	{
		// Clear currently bound framebuffer
		RenderCommandData::ClearData data;
		data.r = 0.0f;
		data.g = 0.0f;
		data.b = 0.0f;
		data.a = 1.0f;
		data.mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;

		commandList.AddControl(w_l_slot, 1, ctrl::Clear, sizeof(data), &data);
	}

	// Do lighting
	commandList.AddDraw(w_l_slot, 0.0f, MaterialId{}, 0);

	// PASS: TRANSPARENT

	// Copy depth buffer

	{
		// Bind geometry framebuffer for reading
		RenderCommandData::BindFramebufferData data;
		data.target = GL_READ_FRAMEBUFFER;
		data.framebuffer = gbuffer.framebuffer;

		commandList.AddControl(w_t_slot, 0, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	{
		// Bind default framebuffer for writing
		RenderCommandData::BindFramebufferData data;
		data.target = GL_DRAW_FRAMEBUFFER;
		data.framebuffer = 0;

		commandList.AddControl(w_t_slot, 1, ctrl::BindFramebuffer, sizeof(data), &data);
	}

	{
		// Blit framebuffer depth from GL_READ_FRAMEBUFFER to GL_DRAW_FRAMEBUFFER
		RenderCommandData::BlitFramebufferData data;

		data.srcLeft = 0;
		data.srcTop = 0;
		data.srcWidth = s.x;
		data.srcHeight = s.y;

		data.dstLeft = 0;
		data.dstTop = 0;
		data.dstWidth = s.x;
		data.dstHeight = s.y;

		data.mask = GL_DEPTH_BUFFER_BIT;
		data.filter = GL_NEAREST;

		commandList.AddControl(w_t_slot, 2, ctrl::BlitFramebuffer, sizeof(data), &data);
	}

	// Before transparent objects in world layer

	// Disable depth writing
	commandList.AddControl(w_t_slot, 3, ctrl::DepthWriteDisable);

	// Enable blending
	commandList.AddControl(w_t_slot, 4, ctrl::BlendingEnable);

	// Create draw commands for render objects in scene
	for (unsigned i = 1; i < data.count; ++i)
	{
		// Object is in potentially visible set
		if (BitPack::Get(data.visibility, i))
		{
			const RenderOrderData& o = data.order[i];

			Vec3f objPosition = (data.transform[i] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

			float depth = Vec3f::Dot(objPosition - cameraPosition, cameraForward) / farPlane;

			RenderCommandSlot slot(o.layer, o.transparency);
			commandList.AddDraw(slot, depth, o.material, i);
		}
	}

	commandList.Sort();
}

void Renderer::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);
	unsigned int visRequired = BitPack::CalculateRequired(required);

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(MeshId) + sizeof(uint64_t) +
		sizeof(RenderOrderData) + sizeof(BoundingBox) + sizeof(Mat4x4f)) +
		visRequired * sizeof(BitPack);

	newData.buffer = operator new[](bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.mesh = reinterpret_cast<MeshId*>(newData.entity + required);
	newData.order = reinterpret_cast<RenderOrderData*>(newData.mesh + required);
	newData.visibility = reinterpret_cast<BitPack*>(newData.order + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.visibility + visRequired);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.mesh, data.mesh, data.count * sizeof(unsigned int));
		std::memcpy(newData.order, data.order, data.count * sizeof(RenderOrderData));
		// Cull state is recalculated every frame
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.transform, data.transform, data.count * sizeof(Mat4x4f));

		operator delete[](data.buffer);
	}

	data = newData;
}

RenderObjectId Renderer::AddRenderObject(Entity entity)
{
	RenderObjectId id;
	this->AddRenderObject(1, &entity, &id);
	return id;
}

void Renderer::AddRenderObject(unsigned int count, Entity* entities, RenderObjectId* renderObjectIdsOut)
{
	if (data.count + count > data.allocated)
		this->Reallocate(data.count + count);

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

void Renderer::NotifyUpdatedTransforms(unsigned int count, Entity* entities, Mat4x4f* transforms)
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
