#include "System/WindowManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "Platform/WindowMetal.hpp"

#include "System/IncludeOpenGL.hpp"
#include "System/InputManager.hpp"
#include "System/WindowSettings.hpp"

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

namespace kokko
{

WindowManager::WindowManager(Allocator* allocator) :
	allocator(allocator),
    window(nullptr),
    currentSwapInterval(-1),
    glfwInitialized(false)
{
}

WindowManager::~WindowManager()
{
    if (window != nullptr)
    {
        allocator->MakeDelete(window);
        window = nullptr;
    }

    if (glfwInitialized)
        glfwTerminate();
}

bool WindowManager::Initialize(const kokko::WindowSettings& settings)
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
        glfwInitialized = true;

#ifdef __APPLE__
        window = allocator->MakeNew<WindowMetal>(allocator);
#else
        window = allocator->MakeNew<WindowOpenGL>(allocator);
#endif

        bool windowInitialized = window->Initialize(settings);

        if (windowInitialized)
			return true;
	}

	KK_LOG_ERROR("GLFW window couldn't be initialized");
	
	return false;
}

void WindowManager::ProcessEvents()
{
	{
		KOKKO_PROFILE_SCOPE("glfwPollEvents()");
		glfwPollEvents();
	}

    window->ProcessEvents();
}

Window* WindowManager::GetWindow()
{
    return window;
}

void WindowManager::SetSwapInterval(int swapInterval)
{
	if (swapInterval != currentSwapInterval)
	{
		currentSwapInterval = swapInterval;
		glfwSwapInterval(swapInterval);
	}
}

int WindowManager::GetSwapInterval() const
{
	return currentSwapInterval;
}


} // namespace kokko
