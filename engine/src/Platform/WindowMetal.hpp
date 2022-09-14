#pragma once

#include "Platform/Window.hpp"

struct GLFWwindow;

namespace kokko
{
class NativeSurface;
struct TextureHandle;

class WindowMetal : public Window
{
public:
    explicit WindowMetal(Allocator* allocator);
    ~WindowMetal();

    GLFWwindow* CreateWindow(const kokko::WindowSettings& settings, NativeRenderDevice* device) override;

    NativeSurface* GetNativeSurface() override;
    TextureHandle GetNativeSurfaceTexture() override;
    void ReleaseNativeSurface() override;

    void Swap() override;

private:
    void* metalLayer;
    void* currentDrawable;
};

}
