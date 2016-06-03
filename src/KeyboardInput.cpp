#include "KeyboardInput.hpp"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

#include "Window.hpp"

KeyboardInput::KeyboardInput() : windowHandle(nullptr), keyStateCount(0)
{

}

KeyboardInput::~KeyboardInput()
{
	if (this->windowHandle != nullptr)
	{
		glfwSetKeyCallback(this->windowHandle, nullptr);
	}
}

void KeyboardInput::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;

	glfwSetKeyCallback(windowHandle, _KeyCallback);
}

void KeyboardInput::Update()
{
	KeyData* itr = keyState;
	KeyData* end = itr + keyStateCount;

	while (itr != end)
	{
		// Up: 0, UpFirst: 1, Down: 2, DownFirst: 3
		int oldState = static_cast<int>(itr->state);

		// Set the least significant bit to zero
		// UpFirst becomes Up, DownFirst becomes Down;
		KeyState newState = static_cast<KeyState>(oldState & 0x2);

		if (newState != Up)
		{
			itr->state = newState;
			++itr;
		}
		else // This entry should be removed
		{
			// Decrease size
			--end;

			// Swap the last item in this place (self-assignment doesn't matter)
			*itr = *end;
		}
	}

	// Set new size to keyStateCount
	keyStateCount = static_cast<unsigned int>(end - keyState);
}

void KeyboardInput::_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	KeyboardInput* self = Window::GetWindowObject(window)->GetKeyboardInput();
	self->KeyCallback(key, scancode, action, mods);
}

void KeyboardInput::KeyCallback(int key, int scancode, int action, int mods)
{
	if (key != GLFW_KEY_UNKNOWN)
	{
		Key keyCode = static_cast<Key>(key);

		KeyData* found = nullptr;
		KeyData* itr = this->keyState;
		KeyData* end = itr + this->keyStateCount;

		for (; itr != end; ++itr)
		{
			if (itr->code == keyCode)
			{
				found = itr;
				break;
			}
		}

		if (found == nullptr && keyStateCount + 1 < maxPressedKeys)
		{
			found = keyState + keyStateCount;
			found->code = keyCode;

			++keyStateCount;
		}

		if (found != nullptr)
		{
			if (action == GLFW_PRESS)
				found->state = DownFirst;
			else if (action == GLFW_RELEASE)
				found->state = UpFirst;
		}
	}
}

bool KeyboardInput::GetKey(Key key) const
{
	const KeyData* itr = this->keyState;
	const KeyData* end = itr + this->keyStateCount;

	for (; itr != end; ++itr)
	{
		if (itr->code == key)
			return (itr->state & 0x2) != 0;
	}

	return false;
}

bool KeyboardInput::GetKeyDown(Key key) const
{
	const KeyData* itr = this->keyState;
	const KeyData* end = itr + this->keyStateCount;

	for (; itr != end; ++itr)
	{
		if (itr->code == key)
			return itr->state == DownFirst;
	}

	return false;
}

bool KeyboardInput::GetKeyUp(Key key) const
{
	const KeyData* itr = this->keyState;
	const KeyData* end = itr + this->keyStateCount;

	for (; itr != end; ++itr)
	{
		if (itr->code == key)
			return itr->state == UpFirst;
	}

	return false;
}
