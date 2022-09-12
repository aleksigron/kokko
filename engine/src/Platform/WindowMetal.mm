#include "Platform/WindowMetal.hpp"

#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "System/WindowSettings.hpp"

namespace kokko
{

WindowMetal::WindowMetal(Allocator* allocator) :
    Window(allocator),
    metalLayer(nullptr)
{
}

WindowMetal::~WindowMetal()
{
}

GLFWwindow* WindowMetal::CreateWindow(const kokko::WindowSettings& settings, NativeRenderDevice* device)
{
    KOKKO_PROFILE_FUNCTION();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
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

    CAMetalLayer* layer = [CAMetalLayer layer];
    layer.device = (__bridge id<MTLDevice>)device;
    layer.opaque = YES;

    NSWindow *nswindow = glfwGetCocoaWindow(window);
    nswindow.contentView.layer = layer;
    nswindow.contentView.wantsLayer = YES;

    metalLayer = layer;

    return window;
}

} // namespace kokko
