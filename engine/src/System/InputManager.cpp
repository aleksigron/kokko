#include "System/InputManager.hpp"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

#include "System/InputSource.hpp"
#include "System/InputView.hpp"

#include "System/IncludeGLFW.hpp"

namespace kokko
{

InputManager::InputManager(Allocator* allocator) :
	windowHandle(nullptr),
	allocator(allocator)
{
}

InputManager::~InputManager()
{
}

void InputManager::Initialize(GLFWwindow* windowHandle)
{
	KOKKO_PROFILE_FUNCTION();

	this->windowHandle = windowHandle;

	inputSource = kokko::MakeUnique<InputSource>(allocator);
	inputSource->Initialize(windowHandle);

	gameInputView = allocator->MakeNew<FilterInputView>(inputSource.Get(), "GameInputView");
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

} // namespace kokko
