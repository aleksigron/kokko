#pragma once

#include "StringRef.hpp"

class DebugTextRenderer;

class DebugLogView
{
private:
	struct LogEntry
	{
		StringRef text;
		int rows;
	};

	DebugTextRenderer* textRenderer;

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

	void AddLogEntry(StringRef text);

	void DrawToTextRenderer();
};
