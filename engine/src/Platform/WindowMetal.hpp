#pragma once

#include "Platform/Window.hpp"

class Allocator;
struct GLFWwindow;

namespace kokko
{

class WindowMetal : public Window
{
public:
    explicit WindowMetal(Allocator* allocator);
    ~WindowMetal();

    virtual bool CreateWindow(const kokko::WindowSettings& settings) override;

private:
    Allocator* allocator;
};

}
