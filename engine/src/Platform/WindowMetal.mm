#include "Platform/WindowMetal.hpp"

#include <QuartzCore/CAMetalLayer.h>
#include <Metal/Metal.h>

#define GLFW_INCLUDE_NONE
#define GLFW_EXPOSE_NATIVE_COCOA
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#include "Rendering/RenderTypes.hpp"

#include "System/WindowSettings.hpp"

namespace kokko
{

WindowMetal::WindowMetal(Allocator* allocator) :
    Window(allocator),
    metalLayer(nullptr),
    currentDrawable(nullptr)
{
}

WindowMetal::~WindowMetal()
{
    [(CAMetalLayer*)metalLayer release];
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
    [(CAMetalLayer*)metalLayer retain];

    return window;
}

NativeSurface* WindowMetal::GetNativeSurface()
{
    if (currentDrawable == nullptr)
    {
        CAMetalLayer* layer = (CAMetalLayer*)metalLayer;
        id<CAMetalDrawable> drawable = [layer nextDrawable];
        [drawable retain];

        currentDrawable = (__bridge void*)drawable;
    }

    return static_cast<NativeSurface*>(currentDrawable);
}

TextureHandle WindowMetal::GetNativeSurfaceTexture()
{
    id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)GetNativeSurface();
    void* ptr = (__bridge void*)[drawable texture];
    return TextureHandle{ reinterpret_cast<uint64_t>(ptr) };
}

void WindowMetal::ReleaseNativeSurface()
{
    if (currentDrawable != nullptr)
    {
        id<CAMetalDrawable> drawable = (__bridge id<CAMetalDrawable>)currentDrawable;
        [drawable release];
        currentDrawable = nullptr;
    }
}

void WindowMetal::Swap()
{
    // NO-OP for now. We could try to do present here at some point.
}

} // namespace kokko
