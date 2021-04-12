#pragma once

#include "Math/Vec2.hpp"

#include "System/KeyCode.hpp"

class InputView
{
private:
	const char* debugName;

	bool blockMouse;
	bool blockKeyboard;
	bool blockText;

	void SetBlockMouseInput(bool blockMouseInput) { blockMouse = blockMouseInput; }
	void SetBlockKeyboardInput(bool blockKeyboardInput) { blockKeyboard = blockKeyboardInput; }
	void SetBlockTextInput(bool blockTextInput) { blockText = blockTextInput; }

	friend class InputManager;

public:
	explicit InputView(const char* debugName) :
		debugName(debugName),
		blockMouse(false),
		blockKeyboard(false),
		blockText(false)
	{
	}

	virtual ~InputView() {}

	virtual Vec2f GetCursorPosition() = 0;
	virtual Vec2f GetCursorMovement() = 0;

	virtual bool GetMouseButton(int buttonIndex) = 0;
	virtual bool GetMouseButtonDown(int buttonIndex) = 0;
	virtual bool GetMouseButtonUp(int buttonIndex) = 0;

	virtual bool GetKey(KeyCode key) = 0;
	virtual bool GetKeyDown(KeyCode key) = 0;
	virtual bool GetKeyUp(KeyCode key) = 0;

	virtual bool WantsMouseInput() { return false; }
	virtual bool WantsKeyboardInput() { return false; }
	virtual bool WantsTextInput() { return false; }

	bool GetBlockMouseInput() { return blockMouse; }
	bool GetBlockKeyboardInput() { return blockKeyboard; }
	bool GetBlockTextInput() { return blockText; }
};

class FilterInputView : public InputView
{
private:
	InputView* source;

public:
	FilterInputView(InputView* sourceInputView, const char* name) :
		InputView(name),
		source(sourceInputView)
	{
	}

	virtual Vec2f GetCursorPosition() override { return source->GetCursorPosition(); }
	virtual Vec2f GetCursorMovement() override { return source->GetCursorMovement(); }

	virtual bool GetMouseButton(int buttonIndex) override
	{
		return !GetBlockMouseInput() && source->GetMouseButton(buttonIndex);
	}

	virtual bool GetMouseButtonDown(int buttonIndex) override
	{
		return !GetBlockMouseInput() && source->GetMouseButtonDown(buttonIndex);
	}

	virtual bool GetMouseButtonUp(int buttonIndex) override
	{
		return !GetBlockMouseInput() && source->GetMouseButtonUp(buttonIndex);
	}

	virtual bool GetKey(KeyCode key) override
	{
		return !GetBlockKeyboardInput() && source->GetKey(key);
	}

	virtual bool GetKeyDown(KeyCode key) override
	{
		return !GetBlockKeyboardInput() && source->GetKeyDown(key);
	}

	virtual bool GetKeyUp(KeyCode key) override
	{
		return !GetBlockKeyboardInput() && source->GetKeyUp(key);
	}
};
