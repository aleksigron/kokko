#pragma once

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugLogView;
class DebugLog;
class KeyboardInput;

class Debug
{
private:
	DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugLogView* logView;
	DebugLog* log;

	KeyboardInput* keyboardInput;

	enum class DebugMode
	{
		None,
		LogView
	}
	mode;

public:
	Debug();
	~Debug();

	void SetKeyboardInput(KeyboardInput* keyboardInput) { this->keyboardInput = keyboardInput; }

	void UpdateLogViewDrawArea();
	
	void Render();

	void CheckOpenGlErrors();

	DebugLog* GetLog() { return log; }
	DebugLogView* GetLogView() { return logView; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
