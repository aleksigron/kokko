#pragma once

#include "StringRef.hpp"
#include "Rectangle.hpp"

class DebugTextRenderer;

class DebugLogView
{
private:
	struct LogEntry
	{
		StringRef text;
		unsigned lengthWithPad;
		int rows;
	};

	DebugTextRenderer* textRenderer;

	Rectangle drawArea;

	LogEntry* entries;
	unsigned int entryFirst;
	unsigned int entryCount;
	unsigned int entryAllocated;

	char* stringData;
	unsigned int stringDataFirst;
	unsigned int stringDataUsed;
	unsigned int stringDataAllocated;

public:
	DebugLogView(DebugTextRenderer* textRenderer);
	~DebugLogView();

	void SetDrawArea(const Rectangle& area);

	void AddLogEntry(StringRef text);

	void DrawToTextRenderer();
};
