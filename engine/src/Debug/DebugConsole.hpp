#pragma once

#include "Core/Queue.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "Math/Rectangle.hpp"

#include "System/LogLevel.hpp"

class Allocator;
class DebugTextRenderer;
class DebugVectorRenderer;

namespace kokko
{
class Window;
}

class DebugConsole
{
private:
	struct LogEntry
	{
		kokko::ConstStringView text;
		size_t lengthWithPad;
		int rows;
		LogLevel level;
	};

	Allocator* allocator;
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
		DebugTextRenderer* textRenderer,
		DebugVectorRenderer* vectorRenderer);
	~DebugConsole();

	void RequestFocus();
	void ReleaseFocus();

	unsigned int GetTotalWarningCount() const { return totalWarningCount; }
	unsigned int GetTotalErrorCount() const { return totalErrorCount; }

	void SetDrawArea(const Rectanglef& area);

	void AddLogEntry(kokko::ConstStringView text, LogLevel level = LogLevel::Info);

	void UpdateAndDraw();
};
