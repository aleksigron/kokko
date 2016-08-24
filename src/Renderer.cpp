#include "Renderer.hpp"

#include <cassert>
#include <cstring>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "App.hpp"
#include "Window.hpp"

#include "Material.hpp"
#include "Shader.hpp"
#include "Texture.hpp"
#include "World.hpp"
#include "Scene.hpp"

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

void Renderer::Render(const World* world, Scene* scene)
{
	ResourceManager* res = App::GetResourceManager();

	RenderObject* o = this->objects;
	BoundingBox* bb = this->boundingBoxes;
	unsigned char* bbcs = this->bboxCullingState;

	Camera* cam = this->activeCamera;

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam);

	this->UpdateBoundingBoxes(scene);

	// Do frustum culling
	FrustumCulling::CullAABB(&frustum, objectCount, bb, bbcs);

	// Get the background color for view
	Color clearCol = world->GetBackgroundColor();
	glClearColor(clearCol.r, clearCol.r, clearCol.r, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthFunc(GL_LESS);

	glDisable(GL_BLEND);

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	// Draw the skybox first
	{
		Mat4x4f cameraRotation = Mat4x4f(cam->transform.rotation.GetTransposed());
		Mat4x4f skyboxTransform = projectionMatrix * cameraRotation;

		const Mesh& mesh = res->meshes.Get(world->GetSkyboxMeshId());
		const Material* skyboxMaterial = res->GetMaterial(world->GetSkyboxMaterialId());
		const Shader* skyboxShader = res->GetShader(skyboxMaterial->shaderId);

		int textureUniformLocation = -1;
		uint32_t textureId = 0;

		for (unsigned int i = 0, count = skyboxMaterial->uniformCount; i < count; ++i)
		{
			const ShaderMaterialUniform& u = skyboxMaterial->uniforms[i];

			if (u.type == ShaderUniformType::TexCube)
			{
				unsigned char* d = skyboxMaterial->uniformData + u.dataOffset;
				textureId = *reinterpret_cast<uint32_t*>(d);

				textureUniformLocation = u.location;
			}
		}

		Texture* skyboxTexture = res->GetTexture(textureId);

		glDisable(GL_DEPTH_TEST);

		glUseProgram(skyboxShader->driverId);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(skyboxTexture->targetType, skyboxTexture->driverId);
		glUniform1i(textureUniformLocation, 0);
		
		glUniformMatrix4fv(skyboxShader->uniformMatVP, 1, GL_FALSE, skyboxTransform.ValuePointer());

		glBindVertexArray(mesh.vertexArrayObject);

		glDrawElements(mesh.primitiveMode, mesh.indexCount, mesh.indexElementType, nullptr);
	}

	glEnable(GL_DEPTH_TEST);

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

			Mat4x4f modelMatrix = scene->GetWorldTransformMatrix(obj.sceneObjectId);

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

			if (shader->uniformMatM >= 0)
			{
				glUniformMatrix4fv(shader->uniformMatM, 1, GL_FALSE, modelMatrix.ValuePointer());
			}

			if (shader->uniformMatV >= 0)
			{
				glUniformMatrix4fv(shader->uniformMatV, 1, GL_FALSE, viewMatrix.ValuePointer());
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
	ResourceManager* rm = App::GetResourceManager();

	for (unsigned i = 0; i < objectCount; ++i)
	{
		const Mesh& mesh = rm->meshes.Get(objects[i].mesh);

		const Mat4x4f& matrix = scene->GetWorldTransformMatrix(objects[i].sceneObjectId);
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
