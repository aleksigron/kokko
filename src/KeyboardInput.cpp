#include "KeyboardInput.hpp"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

#include "App.hpp"

void KeyboardInput::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;
}

void KeyboardInput::Update()
{
	GLFWwindow* w = this->windowHandle;
	unsigned char* state = this->keyState;
	unsigned int count = InternalKeyMap::ForwardSize;
	unsigned int i = 0;
	do
	{
		// Up: 0, UpFirst: 1, Down: 2, DownFirst: 3
		int oldState = state[i];

		// Up: 0, Down: 1
		int keyIsDown = glfwGetKey(w, InternalKeyMap::Forward[i]);

		int keyWasDown = (oldState >> 1); // Up: 0, Down: 1
		int diff = (keyWasDown - keyIsDown) & 1; // Same: 0, different: 1

		state[i] = (keyIsDown << 1) | diff;

		++i;
	}
	while (i < count);
}
