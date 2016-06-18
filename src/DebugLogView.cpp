#include "DebugLogView.hpp"

#include <cstring>

#include "BitmapFont.hpp"
#include "DebugTextRenderer.hpp"

DebugLogView::DebugLogView(DebugTextRenderer* textRenderer) :
	textRenderer(textRenderer),
	entries(nullptr),
	entryFirst(0),
	entryCount(0),
	entryAllocated(0),
	stringData(nullptr),
	stringDataFirst(0),
	stringDataUsed(0),
	stringDataAllocated(0)
{
}

DebugLogView::~DebugLogView()
{
	delete[] stringData;
	delete[] entries;
}

void DebugLogView::AddLogEntry(StringRef text)
{
	Vec2f frameSize = textRenderer->GetScaledFrameSize();

	const BitmapFont* font = textRenderer->GetFont();
	int lineHeight = font->GetLineHeight();

	int screenRows = (static_cast<int>(frameSize.y) + lineHeight - 1) / lineHeight;

	// Check if we have to allocate the buffer
	if (entries == nullptr && stringData == nullptr)
	{
		entryAllocated = screenRows + 1;
		entries = new LogEntry[entryAllocated];

		int rowChars = static_cast<int>(frameSize.x) / font->GetGlyphWidth();

		stringDataAllocated = entryAllocated * rowChars;
		stringData = new char[stringDataAllocated];
	}

	// Check if we have room for a new entry
	if (entryCount < entryAllocated)
	{
		// Add into buffer

		unsigned int newEntryIndex = entryFirst + entryCount;
		++entryCount;

		LogEntry& entry = entries[newEntryIndex % entryAllocated];
		entry.text = StringRef();
		entry.rows = textRenderer->GetRowCountForTextLength(text.len);

		int currentRows = 0;

		for (unsigned int i = 0; i < entryCount; ++i)
		{
			unsigned int index = i % entryAllocated;
			currentRows += entries[index].rows;
		}

		// Check if we can remove
		while (entryCount > 0)
		{
			if (currentRows - entries[entryFirst].rows >= screenRows)
			{
				entryFirst = (entryFirst + 1) % entryAllocated;
				--entryCount;
			}
			else
			{
				break;
			}
		}

		// Copy string data

		unsigned int index = (stringDataFirst + stringDataUsed) % stringDataAllocated;

		if (index + text.len > stringDataAllocated)
		{
			unsigned int padding = stringDataAllocated - index;
			stringDataUsed += padding;

			index = 0;
		}

		if (stringDataUsed + text.len <= stringDataAllocated)
		{
			char* stringLocation = stringData + index;
			entry.text.str = stringLocation;
			entry.text.len = text.len;

			stringDataUsed += text.len;
			std::memcpy(stringLocation, text.str, text.len);
		}
	}
}

void DebugLogView::DrawToTextRenderer()
{
	// Go over each entry
	// Add them to the DebugTextRenderer

	Vec2f frame = textRenderer->GetScaledFrameSize();
	int lineHeight = textRenderer->GetFont()->GetLineHeight();
	int rowsUsed = 0;

	int first = static_cast<int>(entryFirst);
	for (int index = entryFirst + entryCount - 1; index >= first; --index)
	{
		LogEntry& entry = entries[index % entryAllocated];

		rowsUsed += entry.rows;

		Rectangle area;
		area.position.x = 0.0f;
		area.position.y = frame.y - (lineHeight * rowsUsed);
		area.size.x = frame.x;
		area.size.y = lineHeight * rowsUsed;

		textRenderer->AddText(entry.text, area, false);
	}
}




