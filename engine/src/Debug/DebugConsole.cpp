#include "Debug/DebugConsole.hpp"

#include <cstring>

#include "Core/EncodingUtf8.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/Engine.hpp"

#include "Resources/BitmapFont.hpp"

#include "System/InputManager.hpp"
#include "System/InputView.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"

DebugConsole::DebugConsole(
	Allocator* allocator,
	Window* window,
	DebugTextRenderer* textRenderer,
	DebugVectorRenderer* vectorRenderer) :
	allocator(allocator),
	window(window),
	textRenderer(textRenderer),
	vectorRenderer(vectorRenderer),
	entries(allocator),
	stringData(nullptr),
	stringDataFirst(0),
	stringDataUsed(0),
	stringDataAllocated(0),
	totalWarningCount(0),
	totalErrorCount(0),
	inputValue(allocator),
	lastTextInputTime(0)
{
}

DebugConsole::~DebugConsole()
{
	allocator->Deallocate(stringData);
}

void DebugConsole::RequestFocus()
{
}

void DebugConsole::ReleaseFocus()
{
}

void DebugConsole::SetDrawArea(const Rectanglef& area)
{
	this->drawArea = area;
}

void DebugConsole::AddLogEntry(kokko::ConstStringView text, LogLevel level)
{
	const BitmapFont* font = textRenderer->GetFont();

	if (font == nullptr) // Can't do anything reasonable without a font
		return;

	if (drawArea.size.x <= 0.0f || drawArea.size.y <= 0.0f)
		return;

	int lineHeight = font->GetLineHeight();

	Vec2f areaSize = this->drawArea.size;
	int screenRows = static_cast<int>(areaSize.y / lineHeight);

	// Check if we have to allocate the buffer
	if (stringData == nullptr)
	{
		int rowChars = static_cast<int>(areaSize.x) / font->GetGlyphWidth();

		stringDataAllocated = (screenRows + 1) * rowChars;
		stringData = static_cast<char*>(allocator->Allocate(stringDataAllocated));
	}

	// Add into buffer

	size_t charCount = EncodingUtf8::CountCharacters(inputValue.GetRef());

	LogEntry& newEntry = entries.Push();
	newEntry.text = kokko::ConstStringView();
	newEntry.rows = static_cast<int>(textRenderer->GetRowCountForTextLength(charCount));
	newEntry.lengthWithPad = 0;
	newEntry.level = level;

	if (level == LogLevel::Warning)
		totalWarningCount += 1;
	else if (level == LogLevel::Error)
		totalErrorCount += 1;

	int currentRows = 0;

	for (size_t i = 0, count = entries.GetCount(); i < count; ++i)
		currentRows += entries[i].rows;

	// Check if we can remove entries
	while (entries.GetCount() > 0)
	{
		LogEntry& oldEntry = entries[0];

		// We have more than screenRows rows
		if (currentRows - oldEntry.rows >= screenRows)
		{
			stringDataUsed -= oldEntry.lengthWithPad;
			stringDataFirst = (stringDataFirst + oldEntry.lengthWithPad) % stringDataAllocated;

			currentRows -= oldEntry.rows;
			entries.Pop();
		}
		else
		{
			break;
		}
	}

	size_t index = (stringDataFirst + stringDataUsed) % stringDataAllocated;

	// String would overrun end of memory
	if (index + text.len > stringDataAllocated)
	{
		// Pad data to have the string start at the end of memory
		size_t padding = stringDataAllocated - index;
		stringDataUsed += padding;
		newEntry.lengthWithPad += padding;

		index = 0;
	}

	// String can fit in the allocated memory (even with potential padding)
	if (stringDataUsed + text.len <= stringDataAllocated)
	{
		char* stringLocation = stringData + index;
		newEntry.text.str = stringLocation;
		newEntry.text.len = text.len;
		newEntry.lengthWithPad += text.len;

		// Copy string data

		stringDataUsed += text.len;
		std::memcpy(stringLocation, text.str, text.len);
	}
}

void DebugConsole::UpdateAndDraw()
{
	/* *** Update *** */

	InputView* input = window->GetInputManager()->GetGameInputView();

	if (input->GetKeyDown(KeyCode::Enter))
	{
		this->AddLogEntry(this->inputValue.GetRef());
		this->inputValue.Clear();
	}

	if (input->GetKeyDown(KeyCode::Backspace))
	{
		size_t currentLength = this->inputValue.GetLength();

		if (currentLength > 0)
		{
			size_t newLen = EncodingUtf8::FindLastCharacter(this->inputValue.GetRef());

			if (newLen < currentLength)
				this->inputValue.Resize(newLen);
			else // This should only happen if the inputValue somehow becomes invalid UTF-8 text
				this->inputValue.Clear();
		}
	}

	/* **** Draw **** */

	Color white(1.0f, 1.0f, 1.0f);

	const BitmapFont* font = textRenderer->GetFont();
	int lineHeight = font->GetLineHeight();
	Vec2f areaSize = this->drawArea.size;
	Vec2f areaPos = this->drawArea.position;

	// Input text

	Vec2f inputPos(areaPos.x, areaPos.y + areaSize.y - lineHeight);
	textRenderer->AddText(this->inputValue.GetRef(), inputPos);
	
	// Input field separator

	Rectanglef separatorRectangle;
	separatorRectangle.position = inputPos + Vec2f(0.0f, -1.0f);
	separatorRectangle.size = Vec2f(areaSize.x, 1);
	vectorRenderer->DrawRectangleScreen(separatorRectangle, white);

	// Input field caret

	double now = Time::GetRunningTime();
	bool showCaret = std::fmod(now - lastTextInputTime, 0.9) < 0.45;

	if (showCaret)
	{
		size_t chars = EncodingUtf8::CountCharacters(inputValue.GetRef());

		Rectanglef caretRectangle;
		caretRectangle.position.x = static_cast<float>(chars * font->GetGlyphWidth());
		caretRectangle.position.y = inputPos.y;
		caretRectangle.size.x = 1.0f;
		caretRectangle.size.y = static_cast<float>(lineHeight);

		vectorRenderer->DrawRectangleScreen(caretRectangle, white);
	}

	// Go over each entry
	// Add them to the DebugTextRenderer
	int rowsUsed = 1; // 1 row for input

	for (size_t end = entries.GetCount(); end > 0; --end)
	{
		LogEntry& entry = entries[end - 1];

		rowsUsed += entry.rows;

		Rectanglef area;
		area.position.x = areaPos.x;
		area.position.y = areaPos.y + areaSize.y - (lineHeight * rowsUsed);
		area.size.x = areaSize.x;
		area.size.y = static_cast<float>(lineHeight * entry.rows);

		textRenderer->AddText(entry.text, area);
	}
}
