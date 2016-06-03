#include "PointerInput.hpp"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

const int PointerInput::cursorModeValues[] =
{
	GLFW_CURSOR_NORMAL,
	GLFW_CURSOR_HIDDEN,
	GLFW_CURSOR_DISABLED
};

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
	int cursorModeValue = cursorModeValues[static_cast<unsigned int>(mode)];
	glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorModeValue);
}

PointerInput::CursorMode PointerInput::GetCursorMode() const
{
	int cursorModeValue = glfwGetInputMode(windowHandle, GLFW_CURSOR);

	CursorMode mode = CursorMode::Normal;

	if (cursorModeValue == GLFW_CURSOR_HIDDEN)
		mode = CursorMode::Hidden;
	else if (cursorModeValue == GLFW_CURSOR_DISABLED)
		mode = CursorMode::Disabled;

	return mode;
}