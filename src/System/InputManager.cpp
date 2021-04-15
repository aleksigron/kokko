#include "System/InputManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/ImGuiInputView.hpp"
#include "System/InputSource.hpp"
#include "System/InputView.hpp"

#include "System/IncludeGLFW.hpp"

InputManager::InputManager(Allocator* allocator):
	windowHandle(nullptr),
	allocator(allocator),
	inputSource(nullptr),
	imguiInputView(nullptr),
	gameInputView(nullptr)
{
}

InputManager::~InputManager()
{
	allocator->MakeDelete(gameInputView);
	allocator->MakeDelete(imguiInputView);
	allocator->MakeDelete(inputSource);
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	KOKKO_PROFILE_FUNCTION();

	this->windowHandle = windowHandle;

	inputSource = allocator->MakeNew<InputSource>();
	inputSource->Initialize(windowHandle);

	imguiInputView = allocator->MakeNew<ImGuiInputView>(inputSource);
	gameInputView = allocator->MakeNew<FilterInputView>(inputSource, "GameInputView");
}

void InputManager::Update()
{
	KOKKO_PROFILE_FUNCTION();

	inputSource->UpdateInput();
	UpdateInputViews();
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
}

void InputManager::SetCursorMode(CursorMode mode)
{
	int cursorModeValue = GLFW_CURSOR_NORMAL;

	if (mode == CursorMode::Hidden)
		cursorModeValue = GLFW_CURSOR_HIDDEN;
	else if (mode == CursorMode::Disabled)
		cursorModeValue = GLFW_CURSOR_DISABLED;

	glfwSetInputMode(windowHandle, GLFW_CURSOR, cursorModeValue);
}

InputManager::CursorMode InputManager::GetCursorMode() const
{
	int cursorModeValue = glfwGetInputMode(windowHandle, GLFW_CURSOR);

	CursorMode mode = CursorMode::Normal;

	if (cursorModeValue == GLFW_CURSOR_HIDDEN)
		mode = CursorMode::Hidden;
	else if (cursorModeValue == GLFW_CURSOR_DISABLED)
		mode = CursorMode::Disabled;

	return mode;
}
