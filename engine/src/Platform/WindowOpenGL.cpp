#include "Platform/WindowOpenGL.hpp"

#include "Core/Core.hpp"

#include "System/IncludeGLFW.hpp"

namespace kokko
{

bool WindowOpenGL::Initialize()
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

    {
        KOKKO_PROFILE_SCOPE("GLFWwindow* glfwCreateWindow()");
        windowHandle = glfwCreateWindow(settings.width, settings.height, settings.title, NULL, NULL);
    }

    if (windowHandle != nullptr)
    {
        glfwGetFramebufferSize(windowHandle, &currentFramebufferSize.x, &currentFramebufferSize.y);
        glfwGetWindowSize(windowHandle, &currentWindowSize.x, &currentWindowSize.y);
        int maximized = glfwGetWindowAttrib(windowHandle, GLFW_MAXIMIZED);
        currentMaximizeState = maximized == GLFW_TRUE;

        inputManager = allocator->MakeNew<InputManager>(allocator);
        inputManager->Initialize(windowHandle);

        glfwSetWindowUserPointer(windowHandle, this);

        glfwSetFramebufferSizeCallback(windowHandle, _GlfwFramebufferSizeCallback);
        glfwSetWindowSizeCallback(windowHandle, _GlfwWindowSizeCallback);
        glfwSetWindowMaximizeCallback(windowHandle, _GlfwMaximizeCallback);

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
}

}
