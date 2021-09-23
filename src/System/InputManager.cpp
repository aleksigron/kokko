#include "System/InputManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/InputSource.hpp"
#include "System/InputView.hpp"

#include "System/IncludeGLFW.hpp"

InputManager::InputManager(Allocator* allocator):
	windowHandle(nullptr),
	allocator(allocator),
	inputSource(nullptr),
	gameInputView(nullptr)
{
}

InputManager::~InputManager()
{
	allocator->MakeDelete(gameInputView);
	allocator->MakeDelete(inputSource);
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	KOKKO_PROFILE_FUNCTION();

	this->windowHandle = windowHandle;

	inputSource = allocator->MakeNew<InputSource>();
	inputSource->Initialize(windowHandle);

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
}

void InputManager::OnTextInputEnableChanged(bool textInputEnabled)
{
}
