#include "Renderer.hpp"

#include <cassert>
#include <cstring>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Engine.hpp"
#include "App.hpp"
#include "Window.hpp"

#include "ResourceManager.hpp"
#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "World.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

#include "Camera.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "FrustumCulling.hpp"

Renderer::Renderer() :
	indexList(nullptr),
	freeListFirst(0),
	objects(nullptr),
	objectCount(0),
	allocatedCount(0),
	boundingBoxes(nullptr),
	bboxCullingState(nullptr)
{
}

Renderer::~Renderer()
{
	delete[] bboxCullingState;
	delete[] boundingBoxes;
	delete[] objects;
	delete[] indexList;
}

void Renderer::PreTransformUpdate()
{
	Engine* engine = Engine::GetInstance();
	World* world = engine->GetWorld();
	Scene* scene = engine->GetScene();

	Mat4x4f cameraTransform = scene->GetLocalTransform(this->activeCamera->GetSceneObjectId());
	Vec3f cameraPosition = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();

	// Update skybox transform
	world->skybox.UpdateTransform(cameraPosition);
}

void Renderer::Render(const World* world, Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* res = engine->GetResourceManager();

	Camera* cam = this->activeCamera;
	Mat4x4f cameraTransform = scene->GetLocalTransform(cam->GetSceneObjectId());

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam, cameraTransform);

	this->UpdateBoundingBoxes(scene);

	RenderObject* o = this->objects;
	BoundingBox* bb = this->boundingBoxes;
	unsigned char* bbcs = this->bboxCullingState;

	// Do frustum culling
	FrustumCulling::CullAABB(&frustum, objectCount, bb, bbcs);

	// Get the background color for view
	Color clearCol = world->backgroundColor;
	glClearColor(clearCol.r, clearCol.r, clearCol.r, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthFunc(GL_LESS);

	glDisable(GL_BLEND);

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	for (unsigned index = 0; index < objectCount; ++index)
	{
		if (bbcs[index] != 0)
		{
			RenderObject& obj = o[index];

			Mesh& mesh = res->meshes.Get(obj.mesh);
			Material* material = res->GetMaterial(obj.materialId);
			Shader* shader = res->GetShader(material->shaderId);

			glUseProgram(shader->driverId);

			unsigned int usedTextures = 0;

			// Bind each material uniform with a value
			for (unsigned uIndex = 0; uIndex < material->uniformCount; ++uIndex)
			{
				ShaderMaterialUniform& u = material->uniforms[uIndex];

				unsigned char* d = material->uniformData + u.dataOffset;

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
}

void Renderer::AttachTarget(Window* window)
{
	this->targetWindow = window;
}

void Renderer::SetActiveCamera(Camera* camera)
{
	this->activeCamera = camera;
}

void Renderer::UpdateBoundingBoxes(Scene* scene)
{
	Engine* engine = Engine::GetInstance();
	ResourceManager* rm = engine->GetResourceManager();

	for (unsigned i = 0; i < objectCount; ++i)
	{
		const Mesh& mesh = rm->meshes.Get(objects[i].mesh);

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
	bboxCullingState = new unsigned char[newAllocatedCount];

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
		delete[] bboxCullingState;
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
