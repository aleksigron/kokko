#pragma once

struct GLFWwindow;

class TextInputHandler;

class TextInput
{
private:
	GLFWwindow* windowHandle;

	TextInputHandler* currentFocusedHandler;

	static void _TextCallback(GLFWwindow* window, unsigned int codepoint);
	void TextCallback(unsigned int codepoint);

public:
	TextInput();
	~TextInput();

	void Initialize(GLFWwindow* windowHandle);

	bool RequestFocus(TextInputHandler* handler);
	void ReleaseFocus(TextInputHandler* handler);
};
