#include "System/InputSource.hpp"

#include "System/IncludeGLFW.hpp"
#include "System/InputManager.hpp"
#include "System/Window.hpp"

InputSource::InputSource() :
	InputView("InputSource"),
	windowHandle(nullptr),
	keyStateCount(0)
{
	for (unsigned int i = 0; i < MouseButtonCount; ++i)
		mouseButtonState[i] = ButtonState_Down;
}

InputSource::~InputSource()
{
}

void InputSource::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;
}

void InputSource::UpdateInput()
{
	// Mouse position

	Vec2d posd;
	glfwGetCursorPos(windowHandle, &(posd.x), &(posd.y));

	Vec2f posf = posd.As<float>();

	cursorMovement = posf - cursorPosition;
	cursorPosition = posf;

	int i = 0;
	do
	{
		// Up: 0, UpFirst: 1, Down: 2, DownFirst: 3
		int oldState = mouseButtonState[i];

		// Up: 0, Down: 1
		int keyIsDown = glfwGetMouseButton(windowHandle, i);

		int keyWasDown = (oldState >> 1); // Up: 0, Down: 1
		int diff = (keyWasDown - keyIsDown) & 1; // Same: 0, different: 1

		mouseButtonState[i] = static_cast<ButtonState>((keyIsDown << 1) | diff);

		++i;
	}
	while (i < MouseButtonCount);

	// Keyboard

	KeyData* itr = keyState;
	KeyData* end = itr + keyStateCount;

	while (itr != end)
	{
		// Set the least significant bit to zero
		// UpFirst becomes Up, DownFirst becomes Down;
		ButtonState newState = static_cast<ButtonState>(itr->state & 0x2);

		if (newState != ButtonState_Up)
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

Vec2f InputSource::GetCursorPosition()
{
	return cursorPosition;
}

Vec2f InputSource::GetCursorMovement()
{
	return cursorMovement;
}

bool InputSource::GetMouseButton(int buttonIndex)
{
	return (mouseButtonState[buttonIndex] & 0x2) != 0;
}

bool InputSource::GetMouseButtonDown(int buttonIndex)
{
	return mouseButtonState[buttonIndex] == ButtonState_DownFirst;
}

bool InputSource::GetMouseButtonUp(int buttonIndex)
{
	return mouseButtonState[buttonIndex] == ButtonState_UpFirst;
}

bool InputSource::GetKey(KeyCode key)
{
	ButtonState stateOut;
	if (FindKeyState(key, stateOut))
		return (stateOut & 0x2) != 0;

	return false;
}

bool InputSource::GetKeyDown(KeyCode key)
{
	ButtonState stateOut;
	if (FindKeyState(key, stateOut))
		return stateOut == ButtonState_DownFirst;

	return false;
}

bool InputSource::GetKeyUp(KeyCode key)
{
	ButtonState stateOut;
	if (FindKeyState(key, stateOut))
		return stateOut == ButtonState_UpFirst;

	return false;
}

bool InputSource::FindKeyState(KeyCode key, ButtonState& stateOut)
{
	const KeyData* itr = keyState;
	const KeyData* end = itr + keyStateCount;

	for (; itr != end; ++itr)
	{
		if (itr->code == key)
		{
			stateOut = itr->state;
			return true;
		}
	}

	return false;
}

void InputSource::_KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	InputSource* self = Window::GetWindowObject(window)->GetInputManager()->GetInputSource();
	self->KeyCallback(key, scancode, action, mods);
}

void InputSource::KeyCallback(int key, int scancode, int action, int mods)
{
	if (key != GLFW_KEY_UNKNOWN)
	{
		KeyCode keyCode = static_cast<KeyCode>(key);

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

		if (found == nullptr && keyStateCount + 1 < MaxPressedKeys)
		{
			found = keyState + keyStateCount;
			found->code = keyCode;

			++keyStateCount;
		}

		if (found != nullptr)
		{
			if (action == GLFW_PRESS)
				found->state = ButtonState_DownFirst;
			else if (action == GLFW_RELEASE)
				found->state = ButtonState_UpFirst;
		}
	}
}
