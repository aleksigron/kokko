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
	framebufferResizeCallbacks(allocator),
	currentSwapInterval(-1),
	framebufferResizePending(false)
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

			glfwGetFramebufferSize(windowHandle, &currentFramebufferSize.x, &currentFramebufferSize.y);
		}
		
		if (windowHandle != nullptr)
		{
			inputManager = allocator->MakeNew<InputManager>(allocator);
			inputManager->Initialize(windowHandle);

			glfwSetWindowUserPointer(windowHandle, this);

			glfwSetFramebufferSizeCallback(windowHandle, _GlfwFramebufferSizeCallback);

			{
				KOKKO_PROFILE_SCOPE("void glfwMakeContextCurrent()");
				glfwMakeContextCurrent(windowHandle);
			}

			{
				KOKKO_PROFILE_SCOPE("void gladLoadGLLoader()");
				// Tell glad how it can load the OpenGL functions it needs
				gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
			}

			return true;
		}
		
		glfwTerminate();
	}
	
	return false;
}

bool Window::GetShouldClose()
{
	return glfwWindowShouldClose(windowHandle) == GLFW_TRUE;
}

void Window::SetShouldClose(bool shouldClose)
{
	glfwSetWindowShouldClose(windowHandle, shouldClose ? GLFW_TRUE : GLFW_FALSE);
}

void Window::UpdateInput()
{
	KOKKO_PROFILE_FUNCTION();

	if (inputManager != nullptr)
	{
		inputManager->Update();
	}
}

void Window::ProcessEvents()
{
	if (framebufferResizePending)
	{
		for (auto& callback : framebufferResizeCallbacks)
			callback.function(callback.userPointer, this, currentFramebufferSize);

		framebufferResizePending = false;
	}
}

void Window::Swap()
{
	{
		KOKKO_PROFILE_SCOPE("glfwSwapBuffers()");
		glfwSwapBuffers(windowHandle);
	}
	{
		KOKKO_PROFILE_SCOPE("glfwPollEvents()");
		glfwPollEvents();
	}
}

Vec2i Window::GetFrameBufferSize()
{
	return currentFramebufferSize;
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
	if (swapInterval != currentSwapInterval)
	{
		currentSwapInterval = swapInterval;
		glfwSwapInterval(swapInterval);
	}
}

int Window::GetSwapInterval() const
{
	return currentSwapInterval;
}

void Window::SetWindowTitle(const char* title)
{
	glfwSetWindowTitle(windowHandle, title);
}

void Window::SetCursorMode(CursorMode mode)
{
	int cursorModeValue = GLFW_CURSOR_NORMAL;

	if (mode == CursorMode::Hidden)
		cursorModeValue = GLFW_CURSOR_HIDDEN;
	else if (mode == CursorMode::Disabled)
		cursorModeValue = GLFW_CURSOR_DISABLED;

	glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorModeValue);
}

Window::CursorMode Window::GetCursorMode() const
{
	int cursorModeValue = glfwGetInputMode(windowHandle, GLFW_CURSOR);

	CursorMode mode = CursorMode::Normal;

	if (cursorModeValue == GLFW_CURSOR_HIDDEN)
		mode = CursorMode::Hidden;
	else if (cursorModeValue == GLFW_CURSOR_DISABLED)
		mode = CursorMode::Disabled;

	return mode;
}

void Window::RegisterFramebufferResizeCallback(FramebufferSizeCallbackFn callback, void* userPointer)
{
	framebufferResizeCallbacks.PushBack(FramebufferResizeCallbackInfo{ callback, userPointer });
}

void Window::UnregisterFramebufferResizeCallback(FramebufferSizeCallbackFn callback, void* userPointer)
{
	int index = -1;

	for (size_t i = 0, count = framebufferResizeCallbacks.GetCount(); i < count; ++i)
	{
		const FramebufferResizeCallbackInfo& cb = framebufferResizeCallbacks[i];
		if (cb.function == callback && cb.userPointer == userPointer)
		{
			index = static_cast<int>(i);
			break;
		}
	}

	if (index >= 0)
	{
		// Swap the last item into the removed item's place
		if (static_cast<size_t>(index) < framebufferResizeCallbacks.GetCount() - 1)
			framebufferResizeCallbacks[index] = framebufferResizeCallbacks.GetBack();

		framebufferResizeCallbacks.PopBack();
	}
}

Window* Window::GetWindowObject(GLFWwindow* windowHandle)
{
	return static_cast<Window*>(glfwGetWindowUserPointer(windowHandle));
}

void Window::_GlfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	GetWindowObject(window)->GlfwFramebufferSizeCallback(width, height);
}

void Window::GlfwFramebufferSizeCallback(int width, int height)
{
	if ((width != 0 && height != 0) &&
		(width != currentFramebufferSize.x || height != currentFramebufferSize.y))
	{
		framebufferResizePending = true;

		currentFramebufferSize.x = width;
		currentFramebufferSize.y = height;
	}
}
