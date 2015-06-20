#pragma once

#include "Vec2.h"

struct GLFWwindow;

class Window
{
private:
	GLFWwindow* mainWindow = nullptr;
	
public:
	Window();
	~Window();
	
	bool Initialize();
	bool ShouldClose();
	
	Vec2i GetFrameBufferSize();
	
	void TestDraw();
};