#pragma once

#include "System/InputView.hpp"

struct GLFWwindow;

class InputSource final : public InputView
{
private:
	static const int MouseButtonCount = 8;
	static const int MaxPressedKeys = 128;

	enum ButtonState : unsigned char
	{
		ButtonState_Up,
		ButtonState_UpFirst,
		ButtonState_Down,
		ButtonState_DownFirst
	};

	struct KeyData
	{
		KeyCode code;
		ButtonState state;
	};

	GLFWwindow* windowHandle;

	Vec2f cursorPosition;
	Vec2f cursorMovement;

	ButtonState mouseButtonState[MouseButtonCount];
	KeyData keyState[MaxPressedKeys];
	unsigned int keyStateCount;

	bool FindKeyState(KeyCode key, ButtonState& stateOut);

	static void _KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void KeyCallback(int key, int scancode, int action, int mods);

public:
	InputSource();
	virtual ~InputSource();

	void Initialize(GLFWwindow* windowHandle);
	void UpdateInput();

	virtual Vec2f GetCursorPosition() override;
	virtual Vec2f GetCursorMovement() override;

	virtual bool GetMouseButton(int buttonIndex) override;
	virtual bool GetMouseButtonDown(int buttonIndex) override;
	virtual bool GetMouseButtonUp(int buttonIndex) override;

	virtual bool GetKey(KeyCode key) override;
	virtual bool GetKeyDown(KeyCode key) override;
	virtual bool GetKeyUp(KeyCode key) override;
};
