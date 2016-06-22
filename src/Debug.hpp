#pragma once

class DebugTextRenderer;
class DebugLogView;
class DebugLog;
class KeyboardInput;

class Debug
{
private:
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
	Debug(KeyboardInput* keyboardInput);
	~Debug();

	void UpdateLogViewDrawArea();
	
	void Render();

	DebugLog* GetLog() { return log; }
	DebugLogView* GetLogView() { return logView; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
};
