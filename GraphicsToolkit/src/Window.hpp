#pragma once

#include "Vec2.hpp"

struct GLFWwindow;

class Window
{
private:
	GLFWwindow* window = nullptr;
	
public:
	Window();
	~Window();
	
	bool Initialize();
	bool ShouldClose();
	
	void Swap();
	
	Vec2i GetFrameBufferSize();

	GLFWwindow* GetWindowHandle();
};