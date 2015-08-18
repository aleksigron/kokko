#include "KeyboardInput.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

#include "App.h"

void KeyboardInput::Initialize()
{
	this->window = App::GetMainWindow()->GetWindowHandle();
}

bool KeyboardInput::GetKey(Key key)
{
	return glfwGetKey(this->window, static_cast<int>(key)) == GLFW_PRESS;
}
