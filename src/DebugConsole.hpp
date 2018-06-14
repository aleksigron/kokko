#pragma once

#include "String.hpp"
#include "StringRef.hpp"
#include "Rectangle.hpp"
#include "TextInputHandler.hpp"

class DebugTextRenderer;

class DebugConsole : public TextInputHandler
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

	String inputValue;

public:
	DebugConsole(DebugTextRenderer* textRenderer);
	~DebugConsole();

	virtual void OnTextInput(StringRef text);
	void RequestFocus();
	void ReleaseFocus();

	void SetDrawArea(const Rectangle& area);

	void AddLogEntry(StringRef text);

	void DrawToTextRenderer();
};
