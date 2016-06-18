#pragma once

class DebugTextRenderer;
class DebugLogView;
class DebugLog;

class Debug
{
private:
	DebugTextRenderer* textRenderer;
	DebugLogView* logView;
	DebugLog* log;

public:
	Debug();
	~Debug();

	DebugLog* GetLog() { return log; }
	DebugLogView* GetLogView() { return logView; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
};
