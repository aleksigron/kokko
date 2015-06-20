#include "Window.h"

//#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include <iostream>

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
		this->mainWindow = glfwCreateWindow(1280, 720, "Simple example", NULL, NULL);
		
		if (this->mainWindow != nullptr)
		{
			glfwMakeContextCurrent(this->mainWindow);
			glfwSwapInterval(1);
			
			glfwSetKeyCallback(this->mainWindow, GlfwKeyCallback);
			
			return true;
		}
		
		glfwTerminate();
	}
	
	return false;

}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(this->mainWindow);
}

Vec2i Window::GetFrameBufferSize()
{
	Vec2i result;
	
	glfwGetFramebufferSize(this->mainWindow, &result.x, &result.y);
	
	return result;
}

void Window::TestDraw()
{
	Vec2i frameSize = this->GetFrameBufferSize();
	float ratio = frameSize.x / (float)frameSize.y;
	
	glViewport(0, 0, frameSize.x, frameSize.y);
	glClear(GL_COLOR_BUFFER_BIT);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-ratio, ratio, -1.f, 1.f, 1.f, -1.f);
	glMatrixMode(GL_MODELVIEW);
	
	glLoadIdentity();
	glRotatef((float) glfwGetTime() * 50.f, 0.f, 0.f, 1.f);
	
	glBegin(GL_TRIANGLES);
	glColor3f(1.f, 0.f, 0.f);
	glVertex3f(-0.6f, -0.4f, 0.f);
	glColor3f(0.f, 1.f, 0.f);
	glVertex3f(0.6f, -0.4f, 0.f);
	glColor3f(0.f, 0.f, 1.f);
	glVertex3f(0.f, 0.6f, 0.f);
	glEnd();
	
	glfwSwapBuffers(this->mainWindow);
	glfwPollEvents();
}