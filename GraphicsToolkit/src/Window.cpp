#include "Window.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include <iostream>

#include "Vec3.h"

static void GlfwErrorCallback(int error, const char* description)
{
	std::cout << error << ": " << description << std::endl;
}

static void GlfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

Window::Window()
{
}

Window::~Window()
{
	glfwTerminate();
}

bool Window::Initialize()
{
	glfwSetErrorCallback(GlfwErrorCallback);
	
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
			
			glfwSetKeyCallback(this->window, GlfwKeyCallback);
			
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