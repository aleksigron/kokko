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
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "CullState.hpp"
#include "RenderOrder.hpp"

#include "Sort.hpp"

Renderer::Renderer() :
	overrideRenderCamera(nullptr),
	overrideCullingCamera(nullptr)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as RenderObjectId::Null value

	this->Reallocate(512);
}

Renderer::~Renderer()
{
	operator delete[](data.buffer);
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
	FrustumCulling::CullAABB(&frustum, data.count, data.bounds, data.cullState);

	this->CreateDrawCalls(scene);

	// Sort draw calls based on order key
	ShellSortAsc(commands.GetData(), commands.GetCount());

	// Get the background color for view
	RenderPipeline::ClearColorAndDepth(scene->backgroundColor);

	RenderPipeline::DepthTestEnable();
	RenderPipeline::DepthTestFunctionLess();

	RenderPipeline::CullFaceEnable();
	RenderPipeline::CullFaceBack();

	RenderPipeline::BlendingDisable();

	Mat4x4f viewMatrix = Camera::GetViewMatrix(renderCameraTransform);
	Mat4x4f projectionMatrix = renderCamera->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	for (unsigned index = 0, commandCount = commands.GetCount(); index < commandCount; ++index)
	{
		uint64_t command = commands[index];

		// If command is not control command, draw object
		if (pipeline.ParseControlCommand(command) == false)
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
				Mat4x4f vp = projectionMatrix * viewMatrix;
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
				glUniformMatrix4fv(shader->uniformMatP, 1, GL_FALSE, projectionMatrix.ValuePointer());
			}

			MeshDrawData* draw = meshManager->GetDrawData(data.mesh[objIdx]);
			glBindVertexArray(draw->vertexArrayObject);
			
			glDrawElements(draw->primitiveMode, draw->indexCount, draw->indexElementType, nullptr);
		}
	}

	glBindVertexArray(0);

	this->commands.Clear();
}

void Renderer::CreateDrawCalls(Scene* scene)
{
	Camera* renderCamera = this->GetRenderCamera(scene);

	SceneObjectId cameraObject = scene->Lookup(renderCamera->GetEntity());
	Mat4x4f cameraTransform = scene->GetWorldTransform(cameraObject);

	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	Vec3f cameraForward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
	
	float farPlane = renderCamera->farClipDistance;

	// Disable depth writing before objects in skybox layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::Opaque, RenderOrder::Control_DepthWriteDisable));

	// Disable blending before opaque objects in skybox layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::Opaque, RenderOrder::Control_BlendingDisable));

	// Enable blending before transparent objects in skybox layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::TransparentMix, RenderOrder::Control_BlendingEnable));

	// Enable depth writing before objects in world layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::Opaque, RenderOrder::Control_DepthWriteEnable));

	// Disable blending before opaque objects in world layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::Opaque, RenderOrder::Control_BlendingDisable));

	// Enable blending before transparent objects in world layer
	commands.PushBack(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::TransparentMix, RenderOrder::Control_BlendingEnable));

	// Create draw commands for render objects in scene
	for (unsigned i = 1; i < data.count; ++i)
	{
		// Object is in potentially visible set
		if (CullStatePacked16::IsOutside(data.cullState, i) == false)
		{
			const RenderOrderData& o = data.order[i];

			Vec3f objPosition = (data.transform[i] * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

			float depth = Vec3f::Dot(objPosition - cameraPosition, cameraForward) / farPlane;

			if (o.material.IsNull())
			{
				int test = 0;
			}

			commands.PushBack(pipeline.CreateDrawCommand(o.layer, o.transparency, depth, o.material, i));
		}
	}
}

void Renderer::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);
	unsigned int csRequired = CullStatePacked16::CalculateRequired(required);

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	InstanceData newData;
	unsigned int bytes = required * (sizeof(Entity) + sizeof(MeshId) + sizeof(uint64_t) +
		sizeof(RenderOrderData) + sizeof(BoundingBox) + sizeof(Mat4x4f)) +
		csRequired * sizeof(CullStatePacked16);

	newData.buffer = operator new[](bytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.mesh = reinterpret_cast<MeshId*>(newData.entity + required);
	newData.command = reinterpret_cast<uint64_t*>(newData.mesh + required);
	newData.order = reinterpret_cast<RenderOrderData*>(newData.command + required);
	newData.cullState = reinterpret_cast<CullStatePacked16*>(newData.order + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.cullState + csRequired);
	newData.transform = reinterpret_cast<Mat4x4f*>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.mesh, data.mesh, data.count * sizeof(unsigned int));
		std::memcpy(newData.command, data.command, data.count * sizeof(uint64_t));
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
