#pragma once

#include "System/InputView.hpp"

struct GLFWwindow;

namespace kokko
{

class InputSource final : public InputView
{
private:
	static const int MouseButtonCount = 8;
	static const int MaxPressedKeys = 128;
	static const int CharInputBufferSize = 16;

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

	Vec2f scrollPosition;
	Vec2f scrollMovement;
	Vec2d eventScrollOffset;

	ButtonState mouseButtonState[MouseButtonCount];

	KeyData keyState[MaxPressedKeys];
	unsigned int keyStateCount;

	unsigned int eventCharInput[CharInputBufferSize];
	unsigned int eventCharInputCount;

	unsigned int charInput[CharInputBufferSize];
	unsigned int charInputCount;

	bool FindKeyState(KeyCode key, ButtonState& stateOut);

	static void _KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	void KeyCallback(int key, int scancode, int action, int mods);

	static void _ScrollCallback(GLFWwindow* window, double xOffset, double yOffset);
	void ScrollCallback(double xOffset, double yOffset);

	static void _CharCallback(GLFWwindow* window, unsigned int codepoint);
	void CharCallback(unsigned int codepoint);

public:
	InputSource();
	virtual ~InputSource();

	void Initialize(GLFWwindow* windowHandle);
	void UpdateInput();

	virtual Vec2f GetCursorPosition() override;
	virtual Vec2f GetCursorMovement() override;

	virtual Vec2f GetScrollPosition() override;
	virtual Vec2f GetScrollMovement() override;

	virtual bool GetMouseButton(int buttonIndex) override;
	virtual bool GetMouseButtonDown(int buttonIndex) override;
	virtual bool GetMouseButtonUp(int buttonIndex) override;

	virtual bool GetKey(KeyCode key) override;
	virtual bool GetKeyDown(KeyCode key) override;
	virtual bool GetKeyUp(KeyCode key) override;

	virtual int GetActiveKeyCount() override;
	virtual KeyCode GetActiveKeyCode(int index) override;

	virtual int GetInputCharCount() override;
	virtual unsigned int GetInputChar(int index) override;
};

} // namespace kokko
