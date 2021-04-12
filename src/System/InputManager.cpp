#include "System/InputManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/ImGuiInputView.hpp"
#include "System/InputSource.hpp"
#include "System/InputView.hpp"
#include "System/KeyboardInput.hpp"
#include "System/KeyboardInputView.hpp"
#include "System/TextInput.hpp"
#include "System/PointerInput.hpp"

InputManager::InputManager(Allocator* allocator):
	allocator(allocator),
	inputSource(nullptr),
	imguiInputView(nullptr),
	gameInputView(nullptr),
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
	allocator->MakeDelete(gameInputView);
	allocator->MakeDelete(imguiInputView);
	allocator->MakeDelete(inputSource);
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	KOKKO_PROFILE_FUNCTION();

	inputSource = allocator->MakeNew<InputSource>();
	inputSource->Initialize(windowHandle);

	imguiInputView = allocator->MakeNew<ImGuiInputView>(inputSource);
	gameInputView = allocator->MakeNew<FilterInputView>(inputSource, "GameInputView");

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

	inputSource->UpdateInput();
	UpdateInputViews();

	keyboardInput->Update();
	pointerInput->Update();
}

void InputManager::UpdateInputViews()
{
	imguiInputView->SetBlockMouseInput(false);
	imguiInputView->SetBlockKeyboardInput(false);
	imguiInputView->SetBlockTextInput(false);

	gameInputView->SetBlockMouseInput(imguiInputView->WantsMouseInput());
	gameInputView->SetBlockKeyboardInput(imguiInputView->WantsKeyboardInput());
	gameInputView->SetBlockTextInput(imguiInputView->WantsTextInput());
}

void InputManager::OnTextInputEnableChanged(bool textInputEnabled)
{
	keyboardInputView->SetPrintableKeysEnabled(textInputEnabled == false);
}
