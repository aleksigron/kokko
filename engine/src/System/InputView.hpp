#pragma once

#include "Math/Vec2.hpp"

#include "System/KeyCode.hpp"

namespace kokko
{

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

	virtual Vec2f GetScrollPosition() = 0;
	virtual Vec2f GetScrollMovement() = 0;

	virtual bool GetMouseButton(int buttonIndex) = 0;
	virtual bool GetMouseButtonDown(int buttonIndex) = 0;
	virtual bool GetMouseButtonUp(int buttonIndex) = 0;

	virtual bool GetKey(KeyCode key) = 0;
	virtual bool GetKeyDown(KeyCode key) = 0;
	virtual bool GetKeyUp(KeyCode key) = 0;

	virtual int GetActiveKeyCount() = 0;
	virtual KeyCode GetActiveKeyCode(int index) = 0;

	virtual int GetInputCharCount() = 0;
	virtual unsigned int GetInputChar(int index) = 0;

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

	virtual Vec2f GetScrollPosition() override { return source->GetScrollPosition(); }
	virtual Vec2f GetScrollMovement() override { return source->GetScrollMovement(); }

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

	virtual int GetActiveKeyCount() override
	{
		return GetBlockKeyboardInput() ? 0 : source->GetActiveKeyCount();
	}

	virtual KeyCode GetActiveKeyCode(int index) override
	{
		return GetBlockKeyboardInput() ? KeyCode::Unknown : source->GetActiveKeyCode(index);
	}

	virtual int GetInputCharCount() override
	{
		return GetBlockTextInput() ? 0 : source->GetInputCharCount();
	}

	virtual unsigned int GetInputChar(int index) override
	{
		return GetBlockTextInput() ? 0 : source->GetInputChar(index);
	}
};

} // namespace kokko
