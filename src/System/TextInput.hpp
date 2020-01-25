#pragma once

struct GLFWwindow;

class InputManager;
class TextInputHandler;

class TextInput
{
private:
	GLFWwindow* windowHandle;
	InputManager* inputManager;
	TextInputHandler* currentFocusedHandler;

	static void _TextCallback(GLFWwindow* window, unsigned int codepoint);
	void TextCallback(unsigned int codepoint);

public:
	TextInput();
	~TextInput();

	void Initialize(GLFWwindow* windowHandle, InputManager* inputManager);

	bool RequestFocus(TextInputHandler* handler);
	void ReleaseFocus(TextInputHandler* handler);
};
