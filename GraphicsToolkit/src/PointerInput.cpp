#include "PointerInput.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

void PointerInput::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;
}

void PointerInput::Update()
{
	Vec2d posd;
	glfwGetCursorPos(this->windowHandle, &(posd.x), &(posd.y));

	Vec2f posf(static_cast<float>(posd.x), static_cast<float>(posd.y));

	cursorMovement = posf - cursorPosition;
	cursorPosition = posf;
}

void PointerInput::SetCursorMode(CursorMode mode)
{
	glfwSetInputMode(windowHandle, GLFW_CURSOR, static_cast<int>(mode));
}

PointerInput::CursorMode PointerInput::GetCursorMode() const
{
	return static_cast<CursorMode>(glfwGetInputMode(windowHandle, GLFW_CURSOR));
}