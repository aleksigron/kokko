#pragma once

struct GLFWwindow;

class KeyboardInput;
class KeyboardInputView;
class TextInput;
class PointerInput;

class InputSource;
class InputView;

class Allocator;

class InputManager
{
private:
	Allocator* allocator;

	InputSource* inputSource;
	InputView* imguiInputView;
	InputView* gameInputView;

	KeyboardInput* keyboardInput;
	KeyboardInputView* keyboardInputView;

	TextInput* textInput;

	PointerInput* pointerInput;

	void UpdateInputViews();

public:
	InputManager(Allocator* allocator);
	~InputManager();

	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void OnTextInputEnableChanged(bool textInputEnabled);

	InputSource* GetInputSource() { return inputSource; }
	InputView* GetImGuiInputView() { return imguiInputView; }
	InputView* GetGameInputView() { return gameInputView; }

	KeyboardInput* GetKeyboardInput() { return keyboardInput; }
	KeyboardInputView* GetKeyboardInputView() { return keyboardInputView; }
	TextInput* GetTextInput() { return textInput; }
	PointerInput* GetPointerInput() { return pointerInput; }
};
