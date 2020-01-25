#pragma once

struct GLFWwindow;

class KeyboardInput;
class KeyboardInputView;
class TextInput;
class PointerInput;

class InputManager
{
private:
	KeyboardInput* keyboardInput;
	KeyboardInputView* keyboardInputView;

	TextInput* textInput;

	PointerInput* pointerInput;

public:
	InputManager();
	~InputManager();

	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void OnTextInputEnableChanged(bool textInputEnabled);

	KeyboardInput* GetKeyboardInput() { return keyboardInput; }
	KeyboardInputView* GetKeyboardInputView() { return keyboardInputView; }
	TextInput* GetTextInput() { return textInput; }
	PointerInput* GetPointerInput() { return pointerInput; }
};
