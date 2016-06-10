#pragma once

class DebugLog;
class DebugTextRenderer;

class Debug
{
private:
	DebugLog* log;
	DebugTextRenderer* textRenderer;

public:
	Debug();
	~Debug();

	DebugLog* GetLog() { return log; }
	DebugTextRenderer* GetTextRenderer() { return textRenderer; }
};
