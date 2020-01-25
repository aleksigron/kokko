#include "System/KeyboardInputView.hpp"

KeyboardInputView::KeyboardInputView() :
	keyboardInput(nullptr),
	printableKeysEnabled(true)
{
}

KeyboardInputView::~KeyboardInputView()
{
}

void KeyboardInputView::Initialize(KeyboardInput* keyboardInput)
{
	this->keyboardInput = keyboardInput;
}

bool KeyboardInputView::KeyIsPrintable(Key key)
{
	const int first = static_cast<int>(Key::Space);
	const int last = static_cast<int>(Key::World_2);

	int code = static_cast<int>(key);

	return code >= first && code <= last;
}

void KeyboardInputView::SetPrintableKeysEnabled(bool enabled)
{
	printableKeysEnabled = enabled;
}

bool KeyboardInputView::GetKey(Key key) const
{
	return keyboardInput->GetKey(key) && (printableKeysEnabled || KeyIsPrintable(key) == false);
}

bool KeyboardInputView::GetKeyDown(Key key) const
{
	return keyboardInput->GetKeyDown(key) && (printableKeysEnabled || KeyIsPrintable(key) == false);
}

bool KeyboardInputView::GetKeyUp(Key key) const
{
	return keyboardInput->GetKeyUp(key) && (printableKeysEnabled || KeyIsPrintable(key) == false);
}
