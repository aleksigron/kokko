#pragma once

#include "Vec2.h"

struct GLFWwindow;

class PointerInput
{
public:
	enum class CursorMode
	{
		Normal = 0x00034001,
		Hidden = 0x00034002,
		Disabled = 0x00034003
	};

	static const unsigned int mouseButtonCount = 8;

private:
	GLFWwindow* windowHandle;

	Vec2f cursorPosition;
	Vec2f cursorMovement;

	unsigned char mouseButtonState[mouseButtonCount];

	enum ButtonState : unsigned char { Up, UpFirst, Down, DownFirst };

public:
	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void SetCursorMode(CursorMode mode);
	CursorMode GetCursorMode() const;

	inline Vec2f GetCursorPosition() const { return cursorPosition; }
	inline Vec2f GetCursorMovement() const { return cursorMovement; }

	inline bool GetMouseButton(int buttonIndex)
	{ return (mouseButtonState[buttonIndex] & 0x2) != 0; }

	inline bool GetMouseButtonDown(int buttonIndex)
	{ return mouseButtonState[buttonIndex] == DownFirst; }

	inline bool GetMouseButtonUp(int buttonIndex)
	{ return mouseButtonState[buttonIndex] == UpFirst; }
};