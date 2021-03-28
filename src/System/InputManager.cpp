#include "System/InputManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/KeyboardInput.hpp"
#include "System/KeyboardInputView.hpp"
#include "System/TextInput.hpp"
#include "System/PointerInput.hpp"

InputManager::InputManager(Allocator* allocator):
	allocator(allocator),
	keyboardInput(nullptr),
	keyboardInputView(nullptr),
	textInput(nullptr),
	pointerInput(nullptr)
{
}

InputManager::~InputManager()
{
	allocator->MakeDelete(pointerInput);
	allocator->MakeDelete(textInput);
	allocator->MakeDelete(keyboardInputView);
	allocator->MakeDelete(keyboardInput);
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	KOKKO_PROFILE_FUNCTION();

	keyboardInput = allocator->MakeNew<KeyboardInput>();
	keyboardInput->Initialize(windowHandle);

	keyboardInputView = allocator->MakeNew<KeyboardInputView>();
	keyboardInputView->Initialize(keyboardInput);

	textInput = allocator->MakeNew<TextInput>();
	textInput->Initialize(windowHandle, this);

	pointerInput = allocator->MakeNew<PointerInput>();
	pointerInput->Initialize(windowHandle);
}

void InputManager::Update()
{
	KOKKO_PROFILE_FUNCTION();

	keyboardInput->Update();
	pointerInput->Update();
}

void InputManager::OnTextInputEnableChanged(bool textInputEnabled)
{
	keyboardInputView->SetPrintableKeysEnabled(textInputEnabled == false);
}
