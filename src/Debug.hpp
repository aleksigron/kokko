#pragma once

class DebugVectorRenderer;
class DebugTextRenderer;
class DebugGraph;
class DebugLogView;
class DebugLog;

class Window;

class Debug
{
private:
	DebugVectorRenderer* vectorRenderer;
	DebugTextRenderer* textRenderer;
	DebugGraph* graph;
	DebugLogView* logView;
	DebugLog* log;

	Window* window;

	enum class DebugMode
	{
		None,
		LogView,
		FrameTime
	}
	mode;

	void DrawFrameTimeStats();

public:
	Debug();
	~Debug();

	void SetWindow(Window* window);
	
	void Render();

	void CheckOpenGlErrors();

	DebugLog* GetLog() { return log; }
	DebugLogView* GetLogView() { return logView; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
	DebugVectorRenderer* GetVectorRenderer() { return vectorRenderer; }
};
