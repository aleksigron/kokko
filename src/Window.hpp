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
	
	Vec2f GetFrameBufferSize();

	KeyboardInput* GetKeyboardInput() { return keyboardInput; }
	PointerInput* GetPointerInput() { return pointerInput; }
	
	static Window* GetWindowObject(GLFWwindow* windowHandle);
};
