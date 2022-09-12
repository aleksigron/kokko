#pragma once

#include "Platform/Window.hpp"

struct GLFWwindow;

namespace kokko
{

class WindowMetal : public Window
{
public:
    explicit WindowMetal(Allocator* allocator);
    ~WindowMetal();

    virtual GLFWwindow* CreateWindow(const kokko::WindowSettings& settings, NativeRenderDevice* device) override;

private:
    void* metalLayer;
};

}
