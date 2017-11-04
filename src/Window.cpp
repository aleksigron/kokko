#include "Window.hpp"

#include "IncludeOpenGL.hpp"

#include "KeyboardInput.hpp"
#include "PointerInput.hpp"

Window::Window() :
	windowHandle(nullptr),
	keyboardInput(nullptr),
	pointerInput(nullptr),
	currentSwapInterval(0)
{
}

Window::~Window()
{
	glfwTerminate();

	delete pointerInput;
	delete keyboardInput;
}

bool Window::Initialize(const char* windowTitle)
{
	if (glfwInit() == GL_TRUE)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		
		windowHandle = glfwCreateWindow(960, 640, windowTitle, NULL, NULL);
		
		if (windowHandle != nullptr)
		{
			keyboardInput = new KeyboardInput;
			keyboardInput->Initialize(windowHandle);

			pointerInput = new PointerInput;
			pointerInput->Initialize(windowHandle);

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
	if (keyboardInput != nullptr)
	{
		keyboardInput->Update();
	}

	if (pointerInput != nullptr)
	{
		pointerInput->Update();
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

void Window::SetSwapInterval(int swapInterval)
{
	this->currentSwapInterval = swapInterval;
	glfwSwapInterval(swapInterval);
}

Window* Window::GetWindowObject(GLFWwindow* windowHandle)
{
	return static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
}
