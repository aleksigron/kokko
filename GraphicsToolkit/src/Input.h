#pragma once

#include "KeyboardInput.h"

class Input
{
public:
	KeyboardInput keyboard;

	void Initialize() { keyboard.Initialize(); }
	void Update() { keyboard.Update(); }
};