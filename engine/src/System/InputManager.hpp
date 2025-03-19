#pragma once

#include "Core/UniquePtr.hpp"

struct GLFWwindow;

namespace kokko
{

class Allocator;
class InputSource;
class InputView;
class KeyboardInput;
class KeyboardInputView;
class TextInput;
class PointerInput;

class InputManager
{
private:
	GLFWwindow* windowHandle;

	Allocator* allocator;

	UniquePtr<InputSource> inputSource;
	InputView* gameInputView;

	void UpdateInputViews();

public:
	InputManager(Allocator* allocator);
	~InputManager();

	void Initialize(GLFWwindow* windowHandle);
	void Update();

	void OnTextInputEnableChanged(bool textInputEnabled);

	InputSource* GetInputSource() { return inputSource.Get(); }
	InputView* GetGameInputView() { return gameInputView; }
};

} // namespace kokko
