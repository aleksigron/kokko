#pragma once

#include "Vec2.h"

struct GLFWwindow;

class PointerInput
{
private:
	Vec2f cursorPosition;
	Vec2f cursorMovement;

public:
	void Initialize();
	void Update(GLFWwindow* windowHandle);

	inline Vec2f GetCursorPosition() { return cursorPosition; }
	inline Vec2f GetCursorMovement() { return cursorMovement; }
};