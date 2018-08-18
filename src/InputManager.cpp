#include "InputManager.hpp"

#include "KeyboardInput.hpp"
#include "KeyboardInputView.hpp"
#include "TextInput.hpp"
#include "PointerInput.hpp"

InputManager::InputManager()
{
}

InputManager::~InputManager()
{
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	keyboardInput = new KeyboardInput;
	keyboardInput->Initialize(windowHandle);

	keyboardInputView = new KeyboardInputView;
	keyboardInputView->Initialize(keyboardInput);

	textInput = new TextInput;
	textInput->Initialize(windowHandle, this);

	pointerInput = new PointerInput;
	pointerInput->Initialize(windowHandle);
}

void InputManager::Update()
{
	keyboardInput->Update();
	pointerInput->Update();
}

void InputManager::OnTextInputEnableChanged(bool textInputEnabled)
{
	keyboardInputView->SetPrintableKeysEnabled(textInputEnabled == false);
}
