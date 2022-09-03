#include "Platform/WindowMetal.hpp"

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include "System/WindowSettings.hpp"

namespace kokko
{

WindowMetal::WindowMetal(Allocator* allocator) : Window(allocator)
{
}

WindowMetal::~WindowMetal()
{
}

bool WindowMetal::CreateWindow(const kokko::WindowSettings& settings)
{
    KOKKO_PROFILE_FUNCTION();

    const id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    const id<MTLCommandQueue> queue = [device newCommandQueue];
    CAMetalLayer *caLayer = [CAMetalLayer layer];
    caLayer.device = device;
    caLayer.opaque = YES;

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

        return false;
    }

    NSWindow *nswindow = glfwGetCocoaWindow(window);
    nswindow.contentView.layer = caLayer;
    nswindow.contentView.wantsLayer = YES;

    SetGlfwWindow(window);

    return true;
}

} // namespace kokko
