#pragma once

#include "KeyboardInput.hpp"
#include "PointerInput.hpp"

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