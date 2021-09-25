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
	GLFWwindow* windowHandle;

	Allocator* allocator;

	InputSource* inputSource;
	InputView* gameInputView;

	void UpdateInputViews();

public:
	InputManager(Allocator* allocator);
	~InputManager();

	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void OnTextInputEnableChanged(bool textInputEnabled);

	InputSource* GetInputSource() { return inputSource; }
	InputView* GetGameInputView() { return gameInputView; }
};
