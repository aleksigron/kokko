#include "Window.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "Vec2.h"

Window::Window()
{
}

Window::~Window()
{
	glfwTerminate();
}

bool Window::Initialize()
{
	if (glfwInit())
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		this->window = glfwCreateWindow(960, 640, "Simple example", NULL, NULL);
		
		if (this->window != nullptr)
		{
			glfwMakeContextCurrent(this->window);
			glfwSwapInterval(1);

			return true;
		}
		
		glfwTerminate();
	}
	
	return false;

}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(this->window);
}

void Window::Swap()
{
	glfwSwapBuffers(this->window);
	glfwPollEvents();
}

Vec2i Window::GetFrameBufferSize()
{
	Vec2i result;
	
	glfwGetFramebufferSize(this->window, &result.x, &result.y);
	
	return result;
}