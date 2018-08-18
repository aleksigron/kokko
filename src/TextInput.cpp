#include "TextInput.hpp"

#include "IncludeGLFW.hpp"

#include "Window.hpp"
#include "InputManager.hpp"
#include "TextInputHandler.hpp"

#include "StringRef.hpp"
#include "EncodingUtf8.hpp"

TextInput::TextInput() :
	windowHandle(nullptr),
	currentFocusedHandler(nullptr)
{
}

TextInput::~TextInput()
{
	if (this->windowHandle != nullptr)
	{
		glfwSetCharCallback(this->windowHandle, nullptr);
	}
}

void TextInput::Initialize(GLFWwindow* windowHandle, InputManager* inputManager)
{
	this->windowHandle = windowHandle;
	this->inputManager = inputManager;

	glfwSetCharCallback(this->windowHandle, _TextCallback);
}

bool TextInput::RequestFocus(TextInputHandler* handler)
{
	if (this->currentFocusedHandler == nullptr)
		this->inputManager->OnTextInputEnableChanged(true);

	this->currentFocusedHandler = handler;

	return true;
}

void TextInput::ReleaseFocus(TextInputHandler* handler)
{
	if (this->currentFocusedHandler == handler)
	{
		this->currentFocusedHandler = nullptr;
		this->inputManager->OnTextInputEnableChanged(false);
	}
}

void TextInput::_TextCallback(GLFWwindow* window, unsigned int codepoint)
{
	TextInput* self = Window::GetWindowObject(window)->GetInputManager()->GetTextInput();
	self->TextCallback(codepoint);
}

void TextInput::TextCallback(unsigned int codepoint)
{
	if (this->currentFocusedHandler != nullptr)
	{
		char buffer[4];
		unsigned int bytesEncoded = EncodingUtf8::EncodeCodepoint(codepoint, buffer);

		if (bytesEncoded > 0)
			this->currentFocusedHandler->OnTextInput(StringRef(buffer, bytesEncoded));
	}
}
