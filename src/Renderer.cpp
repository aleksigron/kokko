#include "Renderer.hpp"

#include <cassert>
#include <cstring>

#include <cstdio>

#include "IncludeOpenGL.hpp"

#include "Engine.hpp"
#include "App.hpp"
#include "Window.hpp"

#include "ResourceManager.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

#include "Camera.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "FrustumCulling.hpp"
#include "RenderOrder.hpp"

#include "Sort.hpp"

Renderer::Renderer() :
	indexList(nullptr),
	freeListFirst(0),
	objects(nullptr),
	objectCount(0),
	allocatedCount(0),
	boundingBoxes(nullptr),
	cullingState(nullptr)
{
}

Renderer::~Renderer()
{
	delete[] cullingState;
	delete[] boundingBoxes;
	delete[] objects;
	delete[] indexList;
}

void Renderer::PreTransformUpdate(Scene* scene)
{
	Mat4x4f cameraTransform = scene->GetLocalTransform(scene->GetActiveCamera()->GetSceneObjectId());
	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	if (scene->skybox.IsInitialized())
	{
		// Update skybox transform
		scene->skybox.UpdateTransform(cameraPosition);
	}
}

void Renderer::Render(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* res = engine->GetResourceManager();

	Camera* cam = scene->GetActiveCamera();
	Mat4x4f cameraTransform = scene->GetLocalTransform(cam->GetSceneObjectId());

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam, cameraTransform);

	this->UpdateBoundingBoxes(scene);

	// Do frustum culling
	FrustumCulling::CullAABB(&frustum, objectCount, this->boundingBoxes, this->cullingState);

	this->CreateDrawCalls(scene);

	// Sort draw calls based on order key
	ShellSortAsc(commands.GetData(), commands.GetCount());

	// Get the background color for view
	Color clearCol = scene->backgroundColor;
	glClearColor(clearCol.r, clearCol.r, clearCol.r, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	pipeline.DepthTestEnable();
	pipeline.DepthTestFunctionLess();

	pipeline.CullFaceEnable();
	pipeline.CullFaceBack();

	pipeline.BlendingDisable();

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	for (unsigned index = 0, commandCount = commands.GetCount(); index < commandCount; ++index)
	{
		const RenderCommand& command = commands[index];

		// If command is not control command, draw object
		if (pipeline.ParseControlCommand(command.orderKey) == false)
		{
			RenderObject& obj = this->objects[command.renderObjectIndex];

			Mesh& mesh = res->GetMesh(obj.meshId);
			Material& material = res->GetMaterial(obj.materialId);
			Shader* shader = res->GetShader(material.shaderId);

			glUseProgram(shader->driverId);

			unsigned int usedTextures = 0;

			// Bind each material uniform with a value
			for (unsigned uIndex = 0; uIndex < material.uniformCount; ++uIndex)
			{
				ShaderMaterialUniform& u = material.uniforms[uIndex];

				unsigned char* d = material.uniformData + u.dataOffset;

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

			Mat4x4f modelMatrix = scene->GetWorldTransform(obj.sceneObjectId);

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
			
			glBindVertexArray(mesh.vertexArrayObject);
			
			glDrawElements(mesh.primitiveMode, mesh.indexCount, mesh.indexElementType, nullptr);
		}
	}

	glBindVertexArray(0);

	this->commands.Clear();
}

void Renderer::CreateDrawCalls(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* rm = engine->GetResourceManager();

	Camera* activeCamera = scene->GetActiveCamera();
	Mat4x4f cameraTransform = scene->GetWorldTransform(activeCamera->GetSceneObjectId());
	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	Vec3f cameraForward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
	float farPlane = activeCamera->farClipDistance;

	// Disable depth writing before objects in skybox layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::Opaque, RenderOrder::Control_DepthWriteDisable)));

	// Disable blending before opaque objects in skybox layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::Opaque, RenderOrder::Control_BlendingDisable)));

	// Enable blending before transparent objects in skybox layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::Skybox, TransparencyType::TransparentMix, RenderOrder::Control_BlendingEnable)));

	// Enable depth writing before objects in world layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::Opaque, RenderOrder::Control_DepthWriteEnable)));

	// Disable blending before opaque objects in world layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::Opaque, RenderOrder::Control_BlendingDisable)));

	// Enable blending before transparent objects in world layer
	commands.PushBack(RenderCommand(pipeline.CreateControlCommand(
		SceneLayer::World, TransparencyType::TransparentMix, RenderOrder::Control_BlendingEnable)));

	// Create draw commands for render objects in scene
	for (unsigned index = 0; index < objectCount; ++index)
	{
		// Object is in potentially visible set
		if (cullingState[index] != 0)
		{
			RenderObject& obj = objects[index];
			Material& material = rm->GetMaterial(obj.materialId);
			Shader* shader = rm->GetShader(material.shaderId);

			Mat4x4f objTransform = scene->GetWorldTransform(obj.sceneObjectId);
			Vec3f objPosition = (objTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

			float depth = Vec3f::Dot(objPosition - cameraPosition, cameraForward) / farPlane;

			commands.PushBack(RenderCommand(pipeline.CreateDrawCommand(
				obj.layer, shader->transparencyType, depth, obj.materialId), index));
		}
	}
}

void Renderer::UpdateBoundingBoxes(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* rm = engine->GetResourceManager();

	for (unsigned i = 0; i < objectCount; ++i)
	{
		const Mesh& mesh = rm->GetMesh(objects[i].meshId);

		const Mat4x4f& matrix = scene->GetWorldTransform(objects[i].sceneObjectId);
		boundingBoxes[i] = mesh.bounds.Transform(matrix);
	}
}

void Renderer::Reallocate()
{
	unsigned int newAllocatedCount;

	if (allocatedCount == 0)
	{
		newAllocatedCount = 1023;
	}
	else
	{
		newAllocatedCount = allocatedCount * 2 + 1;
	}

	unsigned int* newIndexList = new unsigned int[newAllocatedCount + 1];
	RenderObject* newObjects = new RenderObject[newAllocatedCount];
	boundingBoxes = new BoundingBox[newAllocatedCount];
	cullingState = new unsigned char[newAllocatedCount];

	// We have old data
	if (allocatedCount > 0)
	{
		// Copy old data to new buffers
		std::memcpy(newIndexList, this->indexList, allocatedCount);
		std::memcpy(newObjects, this->objects, allocatedCount);

		// Delete old buffers
		delete[] this->indexList;
		delete[] this->objects;

		// We can delete bounding box data without copying, because it is recreated on every frame
		delete[] boundingBoxes;
		delete[] cullingState;
	}

	this->indexList = newIndexList;
	this->objects = newObjects;

	allocatedCount = newAllocatedCount;
}

unsigned int Renderer::AddRenderObject()
{
	unsigned int id;

	if (freeListFirst == 0)
	{
		if (objectCount == allocatedCount)
		{
			this->Reallocate();
		}

		// If there are no freelist entries, first <objectCount> indices must be in use
		id = objectCount + 1;
		indexList[id] = objectCount;

		++objectCount;
	}
	else
	{
		id = freeListFirst;
		indexList[id] = objectCount;

		freeListFirst = indexList[freeListFirst];
	}

	return id;
}

void Renderer::RemoveRenderObject(unsigned int id)
{
	assert(id != 0);

	// Put last object in removed objects place
	if (indexList[id] < objectCount - 1)
	{
		objects[indexList[id]] = objects[objectCount - 1];
	}

	indexList[id] = freeListFirst;
	freeListFirst = id;

	--objectCount;
}
