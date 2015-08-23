#include "PointerInput.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

void PointerInput::Initialize()
{

}

void PointerInput::Update(GLFWwindow* windowHandle)
{
	Vec2d posd;
	glfwGetCursorPos(windowHandle, &(posd.x), &(posd.y));

	Vec2f posf(static_cast<float>(posd.x), static_cast<float>(posd.y));

	cursorMovement = posf - cursorPosition;
	cursorPosition = posf;
}