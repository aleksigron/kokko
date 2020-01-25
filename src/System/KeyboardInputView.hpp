#pragma once

#include "System/KeyboardInput.hpp"

class KeyboardInput;

class KeyboardInputView
{
private:
	KeyboardInput* keyboardInput;

	bool printableKeysEnabled;

	static bool KeyIsPrintable(Key key);

	void SetPrintableKeysEnabled(bool enabled);

	friend class InputManager;

public:
	KeyboardInputView();
	~KeyboardInputView();

	void Initialize(KeyboardInput* keyboardInput);

	bool GetKey(Key key) const;
	bool GetKeyDown(Key key) const;
	bool GetKeyUp(Key key) const;
};
