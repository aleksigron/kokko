#include "Renderer.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Window.h"
#include "Camera.h"

Renderer::Renderer()
{
}

Renderer::~Renderer()
{
}

void Renderer::Render()
{
	if (this->targetWindow != nullptr && this->activeCamera != nullptr)
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		Mat4x4f viewProjection = this->activeCamera->GetViewProjectionMatrix();
		
		for (const RenderObject& obj : this->renderObjects)
		{
			Mat4x4f mvp = viewProjection * obj.GetTransformMatrix();
			
			GLuint shaderId = obj.GetShaderProgramID();
			
			glUseProgram(shaderId);
			GLuint mvpShaderUniform = glGetUniformLocation(shaderId, "MVP");
			
			glUniformMatrix4fv(mvpShaderUniform, 1, GL_FALSE, mvp.ValuePointer());
			
			glEnableVertexAttribArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, obj.GetVertexBuffer());
			glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
			
			glDrawArrays(GL_TRIANGLES, 0, obj.GetVertexCount());
			
			glDisableVertexAttribArray(0);
		}
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

RenderObject& Renderer::CreateRenderObject()
{
	this->renderObjects.push_front(RenderObject());
	return this->renderObjects.front();
}