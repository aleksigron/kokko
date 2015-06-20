#include "Renderer.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Window.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Render()
{
	if (this->targetWindow != nullptr)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		for (const RenderObject& obj : this->renderObjects)
		{
			glUseProgram(obj.GetShaderProgram());
			
			// 1rst attribute buffer : vertices
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, obj.GetVertexBuffer());
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			
			// Draw the triangle !
			glDrawArrays(GL_TRIANGLES, 0, 3);
			
			glDisableVertexAttribArray(0);
		}
	}
}

void Renderer::AttachTarget(Window* window)
{
	this->targetWindow = window;
}

RenderObject& Renderer::CreateRenderObject()
{
	this->renderObjects.push_front(RenderObject());
	return this->renderObjects.front();
}