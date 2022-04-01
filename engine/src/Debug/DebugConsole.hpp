#pragma once

#include "Core/Queue.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "Debug/LogLevel.hpp"

#include "Math/Rectangle.hpp"

class Allocator;
class Window;
class DebugTextRenderer;
class DebugVectorRenderer;

class DebugConsole
{
private:
	struct LogEntry
	{
		ConstStringView text;
		size_t lengthWithPad;
		int rows;
		LogLevel level;
	};

	Allocator* allocator;
	Window* window;
	DebugTextRenderer* textRenderer;
	DebugVectorRenderer* vectorRenderer;

	Rectanglef drawArea;

	Queue<LogEntry> entries;

	char* stringData;
	size_t stringDataFirst;
	size_t stringDataUsed;
	size_t stringDataAllocated;

	unsigned int totalWarningCount;
	unsigned int totalErrorCount;

	kokko::String inputValue;
	double lastTextInputTime;

public:
	DebugConsole(
		Allocator* allocator,
		Window* window,
		DebugTextRenderer* textRenderer,
		DebugVectorRenderer* vectorRenderer);
	virtual ~DebugConsole();

	void RequestFocus();
	void ReleaseFocus();

	unsigned int GetTotalWarningCount() const { return totalWarningCount; }
	unsigned int GetTotalErrorCount() const { return totalErrorCount; }

	void SetDrawArea(const Rectanglef& area);

	void AddLogEntry(ConstStringView text, LogLevel level = LogLevel::Info);

	void UpdateAndDraw();
};
