#include "Input.hpp"

void Input::Initialize(GLFWwindow* window)
{
	this->windowHandle = window;

	keyboard.Initialize(this->windowHandle);
	pointer.Initialize(this->windowHandle);
}

void Input::Update()
{
	keyboard.Update();
	pointer.Update();
}