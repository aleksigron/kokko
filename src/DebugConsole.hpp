#pragma once

#include "Queue.hpp"
#include "String.hpp"
#include "StringRef.hpp"
#include "Rectangle.hpp"
#include "TextInputHandler.hpp"

class DebugTextRenderer;
class DebugVectorRenderer;

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
	DebugVectorRenderer* vectorRenderer;

	Rectanglef drawArea;

	Queue<LogEntry> entries;

	char* stringData;
	unsigned int stringDataFirst;
	unsigned int stringDataUsed;
	unsigned int stringDataAllocated;

	String inputValue;
	double lastTextInputTime;

public:
	DebugConsole(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer);
	virtual ~DebugConsole();

	virtual void OnTextInput(StringRef text);
	void RequestFocus();
	void ReleaseFocus();

	void SetDrawArea(const Rectanglef& area);

	void AddLogEntry(StringRef text);

	void UpdateAndDraw();
};
