#include "DebugConsole.hpp"

#include <cstring>

#include "Engine.hpp"
#include "Window.hpp"
#include "TextInput.hpp"
#include "BitmapFont.hpp"
#include "DebugTextRenderer.hpp"

DebugConsole::DebugConsole(DebugTextRenderer* textRenderer) :
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

DebugConsole::~DebugConsole()
{
	delete[] stringData;
	delete[] entries;
}

void DebugConsole::OnTextInput(StringRef text)
{
	inputValue.Append(text);
}

void DebugConsole::RequestFocus()
{
	Engine::GetInstance()->GetMainWindow()->GetTextInput()->RequestFocus(this);
}

void DebugConsole::ReleaseFocus()
{
	Engine::GetInstance()->GetMainWindow()->GetTextInput()->ReleaseFocus(this);
}

void DebugConsole::SetDrawArea(const Rectangle& area)
{
	this->drawArea = area;
}

void DebugConsole::AddLogEntry(StringRef text)
{
	Vec2f areaSize = this->drawArea.size;

	const BitmapFont* font = textRenderer->GetFont();
	int lineHeight = font->GetLineHeight();

	int screenRows = areaSize.y / lineHeight;

	// Check if we have to allocate the buffer
	if (entries == nullptr && stringData == nullptr)
	{
		entryAllocated = screenRows + 1;
		entries = new LogEntry[entryAllocated];

		int rowChars = static_cast<int>(areaSize.x) / font->GetGlyphWidth();

		stringDataAllocated = entryAllocated * rowChars;
		stringData = new char[stringDataAllocated];
	}

	// Check if we have room for a new entry
	if (entryCount < entryAllocated)
	{
		// Add into buffer

		unsigned int newEntryIndex = entryFirst + entryCount;
		++entryCount;

		LogEntry& newEntry = entries[newEntryIndex % entryAllocated];
		newEntry.text = StringRef();
		newEntry.rows = textRenderer->GetRowCountForTextLength(text.len);
		newEntry.lengthWithPad = 0;

		int currentRows = 0;

		for (unsigned int i = 0; i < entryCount; ++i)
		{
			unsigned int index = i % entryAllocated;
			currentRows += entries[index].rows;
		}

		// Check if we can remove entries
		while (entryCount > 0)
		{
			LogEntry& oldEntry = entries[entryFirst];

			// We have more than screenRows rows
			if (currentRows > screenRows)
			{
				stringDataUsed -= oldEntry.lengthWithPad;
				stringDataFirst = (stringDataFirst + oldEntry.lengthWithPad) % stringDataAllocated;

				entryFirst = (entryFirst + 1) % entryAllocated;
				--entryCount;

				currentRows -= oldEntry.rows;
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
			newEntry.lengthWithPad += padding;

			index = 0;
		}

		if (stringDataUsed + text.len <= stringDataAllocated)
		{
			char* stringLocation = stringData + index;
			newEntry.text.str = stringLocation;
			newEntry.text.len = text.len;
			newEntry.lengthWithPad += text.len;

			stringDataUsed += text.len;
			std::memcpy(stringLocation, text.str, text.len);
		}
	}
}

void DebugConsole::DrawToTextRenderer()
{
	// input text

	Vec2f position(0.0f, textRenderer->GetFont()->GetLineHeight());
	textRenderer->AddText(this->inputValue.GetRef(), position, true);

	// Go over each entry
	// Add them to the DebugTextRenderer

	Vec2f areaSize = this->drawArea.size;
	Vec2f areaPos = this->drawArea.position;
	int lineHeight = textRenderer->GetFont()->GetLineHeight();
	int rowsUsed = 0;

	int first = static_cast<int>(entryFirst);
	for (int index = entryFirst + entryCount - 1; index >= first; --index)
	{
		LogEntry& entry = entries[index % entryAllocated];

		rowsUsed += entry.rows;

		Rectangle area;
		area.position.x = areaPos.x;
		area.position.y = areaPos.y + areaSize.y - (lineHeight * rowsUsed);
		area.size.x = areaSize.x;
		area.size.y = lineHeight * entry.rows;

		textRenderer->AddText(entry.text, area, false);
	}
}
