#include "Renderer.hpp"

#include <new>
#include <cassert>

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

Renderer::Renderer()
{
	objects = new RenderObject[initialAllocation];
	boundingBoxes = new BoundingBox[initialAllocation];
	bboxCullingState = new unsigned char[initialAllocation];

	allocatedCount = initialAllocation;
}

Renderer::~Renderer()
{
	delete[] bboxCullingState;
	delete[] boundingBoxes;
	delete[] objects;
}

void Renderer::Initialize()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glDepthFunc(GL_LESS);
}

void Renderer::Render(const World* world, Scene* scene)
{
	ResourceManager* res = App::GetResourceManager();

	RenderObject* o = this->objects;
	BoundingBox* bb = this->boundingBoxes;
	unsigned char* bbcs = this->bboxCullingState;

	size_t size = this->contiguousFree;
	unsigned oCount = 0;

	Camera* cam = this->activeCamera;

	// Update bounding boxes
	for (unsigned i = 0; i < size; ++i)
	{
		if (this->RenderObjectIsAlive(i))
		{
			Mesh& mesh = res->meshes.Get(o[i].mesh);

			const Mat4x4f& matrix = scene->GetWorldTransformMatrix(o[i].sceneObjectId);
			bb[oCount] = mesh.bounds.Transform(matrix);

			++oCount;
		}
	}

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam);

	// Do frustum culling
	FrustumCulling::CullAABB(&frustum, oCount, bb, bbcs);

	// Get the background color for view
	Color clearCol = world->GetBackgroundColor();
	glClearColor(clearCol.r, clearCol.r, clearCol.r, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	// Draw the skybox first
	{
		Mat4x4f cameraRotation = Mat4x4f(cam->transform.rotation.GetTransposed());
		Mat4x4f skyboxTransform = projectionMatrix * cameraRotation;

		const Mesh& skyboxMesh = res->meshes.Get(world->GetSkyboxMeshId());
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

		glBindVertexArray(skyboxMesh.vertexArrayObject);

		glDrawElements(GL_TRIANGLES, skyboxMesh.indexCount, skyboxMesh.indexElementType,
					   reinterpret_cast<void*>(0));
	}

	glEnable(GL_DEPTH_TEST);

	for (unsigned arrayIndex = 0, objectIndex = 0; arrayIndex < size; ++arrayIndex)
	{
		if (this->RenderObjectIsAlive(arrayIndex) == true)
		{
			if (bbcs[objectIndex] != 0)
			{
				RenderObject& obj = o[arrayIndex];

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

			++objectIndex;
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

ObjectId Renderer::AddRenderObject()
{
	ObjectId id;
	id.innerId = nextInnerId++;

	if (freeList == UINT_MAX)
	{
		// TODO: Reallocate and copy if contiguousFree == allocatedCount

		// Place the RenderObject into the allocated memory
		RenderObject* o = new (objects + contiguousFree) RenderObject();

		id.index = static_cast<uint32_t>(contiguousFree);
		o->id = id;

		++contiguousFree;
	}
	else
	{
		id.index = freeList;

		/*
		 Get pointer to objects[freeList],
		 cast pointer to char*,
		 add sizeof(RenderObjectId) to pointer value,
		 cast pointer to uint32_t*
		 */

		char* ptr = reinterpret_cast<char*>(&objects[freeList]);
		uint32_t* next = reinterpret_cast<uint32_t*>(ptr + sizeof(ObjectId));
		freeList = *next;
	}

	return id;
}

void Renderer::RemoveRenderObject(ObjectId id)
{
	RenderObject& o = this->GetRenderObject(id);
	o.id.innerId = UINT_MAX;

	/*
	 Get pointer to objects[freeList],
	 cast pointer to char*,
	 add sizeof(RenderObjectId) to pointer value,
	 cast pointer to uint32_t*
	 */

	char* ptr = reinterpret_cast<char*>(&o);
	uint32_t* next = reinterpret_cast<uint32_t*>(ptr + sizeof(ObjectId));
	*next = freeList;
	
	freeList = id.index;
}