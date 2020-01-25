#include "System/InputManager.hpp"

#include "System/KeyboardInput.hpp"
#include "System/KeyboardInputView.hpp"
#include "System/TextInput.hpp"
#include "System/PointerInput.hpp"

InputManager::InputManager():
	keyboardInput(nullptr),
	keyboardInputView(nullptr),
	textInput(nullptr),
	pointerInput(nullptr)
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
