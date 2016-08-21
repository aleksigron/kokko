#pragma once

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugLogView;
class DebugLog;
class KeyboardInput;

struct Camera;

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
	Debug(KeyboardInput* keyboardInput);
	~Debug();

	void UpdateLogViewDrawArea();
	
	void Render(const Camera& camera);

	DebugLog* GetLog() { return log; }
	DebugLogView* GetLogView() { return logView; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
