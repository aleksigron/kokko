#pragma once

#include "Platform/Window.hpp"

namespace kokko
{

class Allocator;

class WindowOpenGL : public Window
{
public:
	WindowOpenGL(Allocator* allocator);
	~WindowOpenGL();

	GLFWwindow* CreateWindow(const WindowSettings& settings, NativeRenderDevice* device) override;

	void SetSwapInterval(int swapInterval) override;
	int GetSwapInterval() const override;
	void Swap() override;

private:
	int currentSwapInterval;
};

}