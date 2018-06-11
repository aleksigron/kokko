#include "TextInput.hpp"

#include "IncludeGLFW.hpp"

#include "TextInputHandler.hpp"
#include "Window.hpp"
#include "StringRef.hpp"

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

void TextInput::Initialize(GLFWwindow* windowHandle)
{
	this->windowHandle = windowHandle;

	glfwSetCharCallback(this->windowHandle, _TextCallback);
}

bool TextInput::RequestFocus(TextInputHandler* handler)
{
	this->currentFocusedHandler = handler;

	return true;
}

void TextInput::ReleaseFocus(TextInputHandler* handler)
{
	if (this->currentFocusedHandler == handler)
		this->currentFocusedHandler = nullptr;
}

void TextInput::_TextCallback(GLFWwindow * window, unsigned int codepoint)
{
	TextInput* self = Window::GetWindowObject(window)->GetTextInput();
	self->TextCallback(codepoint);
}

void TextInput::TextCallback(unsigned int codepoint)
{
	if (this->currentFocusedHandler != nullptr)
	{
		char buffer[4];
		StringRef text;

		if (codepoint <= 0x7F)
		{
			buffer[0] = codepoint;
			text = StringRef(buffer, 1);
		}
		else if (codepoint <= 0x7FF)
		{
			buffer[0] = 0xC0 | codepoint >> 6 & 0x1F;
			buffer[1] = 0x80 | codepoint & 0x3F;
			text = StringRef(buffer, 2);
		}
		else if (codepoint <= 0xFFFF)
		{
			buffer[0] = 0xE0 | codepoint >> 12 & 0xF;
			buffer[1] = 0x80 | codepoint >> 6 & 0x3F;
			buffer[2] = 0x80 | codepoint & 0x3F;
			text = StringRef(buffer, 3);
		}
		else
		{
			buffer[0] = 0xF0 | codepoint >> 18 & 0x7;
			buffer[1] = 0x80 | codepoint >> 12 & 0x3F;
			buffer[2] = 0x80 | codepoint >> 6 & 0x3F;
			buffer[3] = 0x80 | codepoint & 0x3F;
			text = StringRef(buffer, 4);
		}

		this->currentFocusedHandler->OnTextInput(text);
	}
}
