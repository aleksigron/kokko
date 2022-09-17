#include "Platform/WindowOpenGL.hpp"

#include "Core/Core.hpp"

#include "System/IncludeOpenGL.hpp"
#include "System/WindowSettings.hpp"

namespace kokko
{

WindowOpenGL::WindowOpenGL(Allocator* allocator) :
    Window(allocator),
    currentSwapInterval(-1)
{

}

WindowOpenGL::~WindowOpenGL()
{

}

GLFWwindow* WindowOpenGL::CreateWindow(const WindowSettings& settings, NativeRenderDevice* device)
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

    glfwWindowHint(GLFW_MAXIMIZED, settings.maximized ? GLFW_TRUE : GLFW_FALSE);

    GLFWwindow* window;

    {
        KOKKO_PROFILE_SCOPE("GLFWwindow* glfwCreateWindow()");
        window = glfwCreateWindow(settings.width, settings.height, settings.title, NULL, NULL);
    }

    if (window == nullptr)
    {
        KK_LOG_ERROR("GLFW window couldn't be initialized");
        return nullptr;
    }

    {
        KOKKO_PROFILE_SCOPE("glfwMakeContextCurrent()");
        glfwMakeContextCurrent(window);
    }

    {
        KOKKO_PROFILE_SCOPE("gladLoadGLLoader()");
        gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    }

    return window;
}

void WindowOpenGL::SetSwapInterval(int swapInterval)
{
    if (swapInterval != currentSwapInterval)
    {
        currentSwapInterval = swapInterval;
        glfwSwapInterval(swapInterval);
    }
}

int WindowOpenGL::GetSwapInterval() const
{
    return currentSwapInterval;
}

void WindowOpenGL::Swap()
{
    KOKKO_PROFILE_FUNCTION();

    glfwSwapBuffers(GetGlfwWindow());
}

}
