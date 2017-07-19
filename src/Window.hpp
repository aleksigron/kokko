#pragma once

#include "Vec2.hpp"

struct GLFWwindow;

class KeyboardInput;
class PointerInput;

class Window
{
private:
	GLFWwindow* windowHandle;

	KeyboardInput* keyboardInput;
	PointerInput* pointerInput;
	
public:
	Window();
	~Window();
	
	bool Initialize(const char* windowTitle);
	bool ShouldClose();
	void UpdateInput();
	void Swap();
	
	/*
	Get framebuffer size in pixels
	*/
	Vec2f GetFrameBufferSize();

	/*
	Get window content area in screen coordinates
	*/
	Vec2f GetWindowSize();

	/*
	Get screen coordinate scale compared to pixels
	*/
	float GetScreenCoordinateScale();

	KeyboardInput* GetKeyboardInput() { return keyboardInput; }
	PointerInput* GetPointerInput() { return pointerInput; }
	
	static Window* GetWindowObject(GLFWwindow* windowHandle);
};
