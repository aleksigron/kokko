#include "PointerInput.hpp"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

void PointerInput::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;
}

void PointerInput::Update()
{
	GLFWwindow* w = this->windowHandle;

	Vec2d posd;
	glfwGetCursorPos(w, &(posd.x), &(posd.y));

	Vec2f posf(static_cast<float>(posd.x), static_cast<float>(posd.y));

	this->cursorMovement = posf - this->cursorPosition;
	this->cursorPosition = posf;

	int i = 0;
	int count = PointerInput::mouseButtonCount;
	unsigned char* state = this->mouseButtonState;
	do
	{
		// Up: 0, UpFirst: 1, Down: 2, DownFirst: 3
		int oldState = state[i];

		// Up: 0, Down: 1
		int keyIsDown = glfwGetMouseButton(w, i);

		int keyWasDown = (oldState >> 1); // Up: 0, Down: 1
		int diff = (keyWasDown - keyIsDown) & 1; // Same: 0, different: 1

		state[i] = (keyIsDown << 1) | diff;

		++i;

	}
	while (i < count);
}

void PointerInput::SetCursorMode(CursorMode mode)
{
	glfwSetInputMode(windowHandle, GLFW_CURSOR, static_cast<int>(mode));
}

PointerInput::CursorMode PointerInput::GetCursorMode() const
{
	return static_cast<CursorMode>(glfwGetInputMode(windowHandle, GLFW_CURSOR));
}