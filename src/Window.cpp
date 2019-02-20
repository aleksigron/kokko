#include "Window.hpp"

#include "IncludeOpenGL.hpp"

#include "InputManager.hpp"

Window::Window() :
	windowHandle(nullptr),
	inputManager(nullptr),
	currentSwapInterval(0)
{
}

Window::~Window()
{
	glfwTerminate();

	delete inputManager;
}

bool Window::Initialize(const char* windowTitle)
{
	if (glfwInit() == GL_TRUE)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		windowHandle = glfwCreateWindow(1600, 900, windowTitle, NULL, NULL);
		
		if (windowHandle != nullptr)
		{
			inputManager = new InputManager;
			inputManager->Initialize(windowHandle);

			glfwSetWindowUserPointer(windowHandle, this);
			glfwMakeContextCurrent(windowHandle);

			// Tell glad how it can load the OpenGL functions it needs
			gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

			this->SetSwapInterval(1);

			return true;
		}
		
		glfwTerminate();
	}
	
	return false;

}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(windowHandle);
}

void Window::UpdateInput()
{
	if (inputManager != nullptr)
	{
		inputManager->Update();
	}
}

void Window::Swap()
{
	glfwSwapBuffers(windowHandle);
	glfwPollEvents();
}

Vec2f Window::GetFrameBufferSize()
{
	int width, height;
	glfwGetFramebufferSize(windowHandle, &width, &height);

	return Vec2f(width, height);
}

Vec2f Window::GetWindowSize()
{
	int width, height;
	glfwGetWindowSize(windowHandle, &width, &height);

	return Vec2f(width, height);
}

float Window::GetScreenCoordinateScale()
{
	Vec2f pixels = this->GetFrameBufferSize();
	Vec2f screen = this->GetWindowSize();
	
	return pixels.x / screen.x;
}

Mat4x4f Window::GetScreenSpaceProjectionMatrix()
{
	Vec2f frame = this->GetFrameBufferSize();

	const float farClipDistance = 100.0f;
	const float nearClipDistance = -100.0f;

	const float farMinusNear = farClipDistance - nearClipDistance;
	const float farPlusNear = farClipDistance + nearClipDistance;

	Mat4x4f result;
	result[0] = 2.0f / frame.x;
	result[5] = -2.0f / frame.y;
	result[10] = -2.0f / (farMinusNear);
	result[12] = -1.0f;
	result[13] = 1.0f;
	result[14] = -(farPlusNear) / (farMinusNear);

	return result;
}

void Window::SetSwapInterval(int swapInterval)
{
	this->currentSwapInterval = swapInterval;
	glfwSwapInterval(swapInterval);
}

Window* Window::GetWindowObject(GLFWwindow* windowHandle)
{
	return static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
}
