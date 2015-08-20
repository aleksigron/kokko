#include "KeyboardInput.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

#include "App.h"

void KeyboardInput::Initialize()
{
	this->window = App::GetMainWindow()->GetWindowHandle();
}

void KeyboardInput::Update()
{
	unsigned int count = InternalKeyMap::ForwardSize;
	unsigned int i = 0;
	do
	{
		// Up: 0, UpFirst: 1, Down: 2, DownFirst: 3
		int oldState = keyState[i];

		// Up: 0, Down: 1
		int keyIsDown = glfwGetKey(window, InternalKeyMap::Forward[i]);

		int keyWasDown = (oldState >> 1); // Up: 0, Down: 1
		int diff = (keyWasDown - keyIsDown) & 1; // Same: 0, different: 1

		keyState[i] = (keyIsDown << 1) | diff;

		++i;
	}
	while (i < count);
}
