#include "Input.h"

void Input::Initialize(GLFWwindow* window)
{
	this->windowHandle = window;

	keyboard.Initialize();
	pointer.Initialize();
}

void Input::Update()
{
	keyboard.Update(windowHandle);
	pointer.Update(windowHandle);
}