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

private:
	GLFWwindow* windowHandle;

	Vec2f cursorPosition;
	Vec2f cursorMovement;

public:
	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void SetCursorMode(CursorMode mode);
	CursorMode GetCursorMode() const;

	inline Vec2f GetCursorPosition() const { return cursorPosition; }
	inline Vec2f GetCursorMovement() const { return cursorMovement; }
};