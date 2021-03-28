#include "System/Window.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/IncludeOpenGL.hpp"
#include "System/InputManager.hpp"

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

Window::Window(Allocator* allocator) :
	allocator(allocator),
	windowHandle(nullptr),
	inputManager(nullptr),
	currentSwapInterval(0)
{
}

Window::~Window()
{
	allocator->MakeDelete(inputManager);

	if (windowHandle != nullptr)
	{
		glfwDestroyWindow(windowHandle);
		glfwTerminate();
	}
}

bool Window::Initialize(int width, int height, const char* windowTitle)
{
	KOKKO_PROFILE_FUNCTION();

	glfwSetErrorCallback(OnGlfwError);

	int initResult;

	{
		KOKKO_PROFILE_SCOPE("int glfwInit()");
		initResult = glfwInit();
	}

	if (initResult == GLFW_TRUE)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
		glfwWindowHint(GLFW_ALPHA_BITS, 0);
		glfwWindowHint(GLFW_DEPTH_BITS, 0);
		glfwWindowHint(GLFW_STENCIL_BITS, 0);

		{
			KOKKO_PROFILE_SCOPE("GLFWwindow* glfwCreateWindow()");
			windowHandle = glfwCreateWindow(width, height, windowTitle, NULL, NULL);
		}
		
		if (windowHandle != nullptr)
		{
			inputManager = allocator->MakeNew<InputManager>(allocator);
			inputManager->Initialize(windowHandle);

			glfwSetWindowUserPointer(windowHandle, this);

			{
				KOKKO_PROFILE_SCOPE("void glfwMakeContextCurrent()");
				glfwMakeContextCurrent(windowHandle);
			}

			{
				KOKKO_PROFILE_SCOPE("void gladLoadGLLoader()");
				// Tell glad how it can load the OpenGL functions it needs
				gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			}

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
	KOKKO_PROFILE_FUNCTION();

	if (inputManager != nullptr)
	{
		inputManager->Update();
	}
}

void Window::Swap()
{
	KOKKO_PROFILE_FUNCTION();

	glfwSwapBuffers(windowHandle);
	glfwPollEvents();
}

Vec2i Window::GetFrameBufferSize()
{
	int width, height;
	glfwGetFramebufferSize(windowHandle, &width, &height);

	return Vec2i(width, height);
}

Vec2i Window::GetWindowSize()
{
	int width, height;
	glfwGetWindowSize(windowHandle, &width, &height);

	return Vec2i(width, height);
}

float Window::GetScreenCoordinateScale()
{
	Vec2i pixels = this->GetFrameBufferSize();
	Vec2i screen = this->GetWindowSize();
	
	return pixels.x / static_cast<float>(screen.x);
}

Mat4x4f Window::GetScreenSpaceProjectionMatrix()
{
	Vec2i frame = this->GetFrameBufferSize();

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
