#include "System/WindowManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "Platform/Window.hpp"

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
    glfwInitialized(false)
{
}

WindowManager::~WindowManager()
{
    if (window != nullptr)
        allocator->MakeDelete(window);

    if (glfwInitialized)
        glfwTerminate();
}

bool WindowManager::Initialize(const kokko::WindowSettings& settings, NativeRenderDevice* device)
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

        window = Window::Create(allocator);

        bool windowInitialized = window->Initialize(settings, device);

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


} // namespace kokko
