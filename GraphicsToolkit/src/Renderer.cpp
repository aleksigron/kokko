#include "Renderer.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include <new>
#include <cassert>

#include "Window.h"
#include "Camera.h"
#include "App.h"

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
	if (this->targetWindow != nullptr && this->activeCamera != nullptr)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Mat4x4f viewProjection = this->activeCamera->GetViewProjectionMatrix();
		
		for (size_t i = 0; i < contiguousFree; ++i)
		{
			RenderObject& obj = objects[i];
			
			Mat4x4f mvp = viewProjection * obj.transform.GetTransformMatrix();

			ResourceManager* res = App::GetResourceManager();
			ShaderProgram& shader = res->shaders.Get(obj.shader);

			glUseProgram(shader.oglId);

			glUniformMatrix4fv(shader.mvpUniformLocation, 1,
							   GL_FALSE, mvp.ValuePointer());

			if (obj.hasTexture) // HORRIBLE & UGLY
			{
				Texture& texture = res->textures.Get(obj.texture);

				// Bind our texture in Texture Unit 0
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, texture.textureGlId);

				GLint texUniform = glGetUniformLocation(shader.oglId, "tex");

				// Set texture uniform to texture unit 0
				glUniform1i(texUniform, 0);
			}

			glBindVertexArray(obj.vertexArrayObject);
			
			glDrawElements(GL_TRIANGLES, obj.indexCount,
						   obj.indexElementType, reinterpret_cast<void*>(0));
		}

		glBindVertexArray(0);
	}
}

void Renderer::AttachTarget(Window* window)
{
	this->targetWindow = window;
}

void Renderer::SetActiveCamera(Camera* camera)
{
	this->activeCamera = camera;
}

bool Renderer::HasRenderObject(ObjectId id)
{
	return objects[id.index].id.innerId == id.innerId;
}

RenderObject& Renderer::GetRenderObject(ObjectId id)
{
	return objects[id.index];
}

ObjectId Renderer::AddRenderObject()
{
	ObjectId id;
	id.innerId = nextInnerId++;
	
	if (freeList == UINT32_MAX)
	{
		// TODO: Reallocate and copy if contiguousFree == allocatedCount
		
		// Placement new the RenderObject into the allocated memory
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
	o.id.innerId = UINT32_MAX;

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