#include "Debug/DebugConsole.hpp"

#include <cstring>

#include "Engine.hpp"
#include "Time.hpp"

#include "Window.hpp"
#include "InputManager.hpp"
#include "KeyboardInputView.hpp"
#include "TextInput.hpp"
#include "EncodingUtf8.hpp"
#include "BitmapFont.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

DebugConsole::DebugConsole(DebugTextRenderer* textRenderer, DebugVectorRenderer* vectorRenderer) :
	textRenderer(textRenderer),
	vectorRenderer(vectorRenderer),
	stringData(nullptr),
	stringDataFirst(0),
	stringDataUsed(0),
	stringDataAllocated(0),
	lastTextInputTime(0)
{
}

DebugConsole::~DebugConsole()
{
	delete[] stringData;
}

void DebugConsole::OnTextInput(StringRef text)
{
	inputValue.Append(text);
	lastTextInputTime = Time::GetRunningTime();
}

void DebugConsole::RequestFocus()
{
	Engine::GetInstance()->GetMainWindow()->GetInputManager()->GetTextInput()->RequestFocus(this);
}

void DebugConsole::ReleaseFocus()
{
	Engine::GetInstance()->GetMainWindow()->GetInputManager()->GetTextInput()->ReleaseFocus(this);
}

void DebugConsole::SetDrawArea(const Rectanglef& area)
{
	this->drawArea = area;
}

void DebugConsole::AddLogEntry(StringRef text)
{
	const BitmapFont* font = textRenderer->GetFont();

	if (font == nullptr) // Can't do anything reasonable without a font
		return;

	int lineHeight = font->GetLineHeight();

	Vec2f areaSize = this->drawArea.size;
	int screenRows = areaSize.y / lineHeight;

	// Check if we have to allocate the buffer
	if (stringData == nullptr)
	{
		int rowChars = static_cast<int>(areaSize.x) / font->GetGlyphWidth();

		stringDataAllocated = (screenRows + 1) * rowChars;
		stringData = new char[stringDataAllocated];
	}

	// Add into buffer

	unsigned int charCount = EncodingUtf8::CountCharacters(inputValue.GetRef());

	LogEntry& newEntry = entries.Push();
	newEntry.text = StringRef();
	newEntry.rows = textRenderer->GetRowCountForTextLength(charCount);
	newEntry.lengthWithPad = 0;

	int currentRows = 0;

	for (unsigned int i = 0, count = entries.GetCount(); i < count; ++i)
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

	unsigned int index = (stringDataFirst + stringDataUsed) % stringDataAllocated;

	// String would overrun end of memory
	if (index + text.len > stringDataAllocated)
	{
		// Pad data to have the string start at the end of memory
		unsigned int padding = stringDataAllocated - index;
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

	KeyboardInputView* kiv = Engine::GetInstance()->GetMainWindow()->GetInputManager()->GetKeyboardInputView();

	if (kiv->GetKeyDown(Key::Enter))
	{
		this->AddLogEntry(this->inputValue.GetRef());
		this->inputValue.Clear();
	}

	if (kiv->GetKeyDown(Key::Backspace))
	{
		unsigned currentLength = this->inputValue.GetLength();

		if (currentLength > 0)
		{
			unsigned int newLen = EncodingUtf8::FindLastCharacter(this->inputValue.GetRef());

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
		int chars = EncodingUtf8::CountCharacters(inputValue.GetRef());

		Rectanglef caretRectangle;
		caretRectangle.position = Vec2f(chars * font->GetGlyphWidth(), inputPos.y);
		caretRectangle.size = Vec2f(1, lineHeight);

		vectorRenderer->DrawRectangleScreen(caretRectangle, white);
	}

	// Go over each entry
	// Add them to the DebugTextRenderer
	int rowsUsed = 1; // 1 row for input

	for (int i = entries.GetCount() - 1; i >= 0; --i)
	{
		LogEntry& entry = entries[i];

		rowsUsed += entry.rows;

		Rectanglef area;
		area.position.x = areaPos.x;
		area.position.y = areaPos.y + areaSize.y - (lineHeight * rowsUsed);
		area.size.x = areaSize.x;
		area.size.y = lineHeight * entry.rows;

		textRenderer->AddText(entry.text, area);
	}
}
