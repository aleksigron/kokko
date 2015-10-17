#pragma once

#include "KeyboardInput.h"
#include "PointerInput.h"

struct GLFWwindow;

class Input
{
private:
	GLFWwindow* windowHandle;

public:
	KeyboardInput keyboard;
	PointerInput pointer;

	void Initialize(GLFWwindow* window);
	void Update();
};