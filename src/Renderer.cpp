#include "Renderer.hpp"

#include <new>
#include <cassert>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Window.hpp"
#include "Camera.hpp"
#include "App.hpp"
#include "Material.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "Scene.hpp"

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

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Renderer::Render(Scene& scene)
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

			const Mat4x4f& matrix = scene.GetWorldTransformMatrix(o[i].sceneObjectId);
			bb[oCount] = mesh.bounds.Transform(matrix);

			++oCount;
		}
	}

	// Update view frustum
	ViewFrustum frustum;
	frustum.UpdateFrustum(*cam);

	// Do frustum culling
	frustum.CullAABB(oCount, bb, bbcs);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Mat4x4f viewMatrix = cam->GetViewMatrix();
	Mat4x4f projectionMatrix = cam->GetProjectionMatrix();
	Mat4x4f viewProjection = projectionMatrix * viewMatrix;

	for (unsigned arrayIndex = 0, objectIndex = 0; arrayIndex < size; ++arrayIndex)
	{
		if (this->RenderObjectIsAlive(arrayIndex) == true)
		{
			if (bbcs[objectIndex] != 0)
			{
				RenderObject& obj = o[arrayIndex];

				Mesh& mesh = res->meshes.Get(obj.mesh);
				Material* material = res->GetMaterial(obj.materialId);
				ShaderProgram* shader = res->GetShader(material->shaderId);

				glUseProgram(shader->driverId);

				// Bind each material uniform with a value
				for (unsigned uIndex = 0; uIndex < material->uniformCount; ++uIndex)
				{
					ShaderMaterialUniform& u = material->uniforms[uIndex];

					unsigned char* uData = material->uniformData + u.dataOffset;

					switch (u.type)
					{
					case ShaderUniformType::Mat4x4:
						glUniformMatrix4fv(u.location, 1, GL_FALSE,
										   reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Vec4:
						glUniform4fv(u.location, 1, reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Vec3:
						glUniform3fv(u.location, 1, reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Vec2:
						glUniform2fv(u.location, 1, reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Float:
						glUniform1f(u.location, *reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Int:
						glUniform1i(u.location, *reinterpret_cast<int*>(uData));
						break;

					case ShaderUniformType::Tex2D:
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, *reinterpret_cast<int*>(uData));
						glUniform1i(u.location, 0);
						break;
					}
				}

				Mat4x4f modelMatrix = scene.GetWorldTransformMatrix(obj.sceneObjectId);

				if (shader->uniformMVP >= 0)
				{
					Mat4x4f mvp = viewProjection * modelMatrix;
					glUniformMatrix4fv(shader->uniformMVP, 1, GL_FALSE, mvp.ValuePointer());
				}

				if (shader->uniformMV >= 0)
				{
					Mat4x4f mv = viewMatrix * modelMatrix;
					glUniformMatrix4fv(shader->uniformMV, 1, GL_FALSE, mv.ValuePointer());
				}

				glBindVertexArray(mesh.vertexArrayObject);

				glDrawElements(GL_TRIANGLES, mesh.indexCount,
							   mesh.indexElementType, reinterpret_cast<void*>(0));
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