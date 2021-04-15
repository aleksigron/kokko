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
public:
	enum class CursorMode
	{
		Normal,
		Hidden,
		Disabled
	};

private:
	GLFWwindow* windowHandle;

	Allocator* allocator;

	InputSource* inputSource;
	InputView* imguiInputView;
	InputView* gameInputView;

	void UpdateInputViews();

public:
	InputManager(Allocator* allocator);
	~InputManager();

	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void OnTextInputEnableChanged(bool textInputEnabled);

	void SetCursorMode(CursorMode mode);
	CursorMode GetCursorMode() const;

	InputSource* GetInputSource() { return inputSource; }
	InputView* GetImGuiInputView() { return imguiInputView; }
	InputView* GetGameInputView() { return gameInputView; }
};
