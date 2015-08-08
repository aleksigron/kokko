#include "Renderer.h"

#include <new>
#include <cassert>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Window.h"
#include "Camera.h"
#include "App.h"
#include "Material.h"

Renderer::Renderer()
{
	objects = new RenderObject[initialAllocation];
	allocatedCount = initialAllocation;
}

Renderer::~Renderer()
{
	delete[] objects;
}

void Renderer::Initialize()
{
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
}

void Renderer::Render()
{
	assert(targetWindow != nullptr);
	assert(activeCamera != nullptr);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ResourceManager* res = App::GetResourceManager();

	Mat4x4f viewProjection = this->activeCamera->GetViewProjectionMatrix();

	for (unsigned i = 0; i < contiguousFree; ++i)
	{
		if (this->RenderObjectIsAlive(i) == true)
		{
			RenderObject& obj = objects[i];

			Material& material = res->materials.Get(obj.material);
			ShaderProgram& shader = res->shaders.Get(material.shader);

			glUseProgram(shader.oglId);

			// Bind each material uniform with a value
			for (unsigned uIndex = 0; uIndex < material.uniformCount; ++uIndex)
			{
				ShaderMaterialUniform& u = material.uniforms[uIndex];

				unsigned char* uData = material.uniformData + u.dataOffset;

				switch (u.type)
				{
					case ShaderUniformType::Mat4x4:
						glUniformMatrix4fv(u.location, 1, GL_FALSE,
										   reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Vec3:
						glUniform3fv(u.location, 1,
									 reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Vec2:
						glUniform2fv(u.location, 1,
									 reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Float:
						glUniform1f(u.location,
									*reinterpret_cast<float*>(uData));
						break;

					case ShaderUniformType::Texture2D:
						glActiveTexture(GL_TEXTURE0);
						glBindTexture(GL_TEXTURE_2D, *reinterpret_cast<int*>(uData));
						glUniform1i(u.location, 0);
						break;
				}
			}

			Mat4x4f mvp = viewProjection * obj.transform.GetTransformMatrix();

			glUniformMatrix4fv(shader.mvpUniformLocation, 1,
							   GL_FALSE, mvp.ValuePointer());

			glBindVertexArray(obj.vertexArrayObject);

			glDrawElements(GL_TRIANGLES, obj.indexCount,
						   obj.indexElementType, reinterpret_cast<void*>(0));

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