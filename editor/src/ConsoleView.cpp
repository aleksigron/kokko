#include "ConsoleView.hpp"

#include <cstring>

#include "imgui.h"

#include "Core/EncodingUtf8.hpp"

#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Engine/Engine.hpp"

#include "Resources/BitmapFont.hpp"

#include "System/InputManager.hpp"
#include "System/InputView.hpp"
#include "System/Time.hpp"
#include "Platform/Window.hpp"

#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

ConsoleView::ConsoleView(Allocator* allocator) :
	EditorWindow("Console"),
	allocator(allocator),
	entries(allocator),
	stringData(nullptr),
	stringDataFirst(0),
	stringDataUsed(0),
	stringDataAllocated(0),
	totalWarningCount(0),
	totalErrorCount(0)
{

	memset(inputBuffer, 0, sizeof(inputBuffer));
}

ConsoleView::~ConsoleView()
{
	allocator->Deallocate(stringData);
}

void ConsoleView::AddLogEntry(kokko::ConstStringView text, LogLevel level)
{
	// Check if we have to allocate the buffer
	if (stringData == nullptr)
	{
		stringDataAllocated = 1 << 16; // 64k
		stringData = static_cast<char*>(allocator->Allocate(stringDataAllocated, "ConsoleView.stringData"));
	}

	// Add into buffer

	size_t byteCount = text.len + 1; // +1 null terminator

	// Check if we need to remove entries
	while (entries.GetCount() > 0)
	{
		LogEntry& oldEntry = entries[0];

		// We wouldn't fit all the text into the buffer
		if (stringDataUsed + byteCount > stringDataAllocated)
		{
			stringDataUsed -= oldEntry.lengthWithPad;
			stringDataFirst = (stringDataFirst + oldEntry.lengthWithPad) % stringDataAllocated;

			entries.Pop();
		}
		else
		{
			break;
		}
	}


	size_t index = (stringDataFirst + stringDataUsed) % stringDataAllocated;
	size_t padding = 0;

	// String would overrun end of memory
	if (index + text.len > stringDataAllocated)
	{
		// Pad data to have the string start at the end of memory
		padding = stringDataAllocated - index;

		index = 0;
	}

	size_t lengthWithPad = byteCount + padding;

	if (level == LogLevel::Warning)
		totalWarningCount += 1;
	else if (level == LogLevel::Error)
		totalErrorCount += 1;

	// String can fit in the allocated memory (even with potential padding)
	if (stringDataUsed + lengthWithPad <= stringDataAllocated)
	{
		char* stringLocation = stringData + index;
		LogEntry& newEntry = entries.Push();
		newEntry.text = stringLocation;
		newEntry.length = text.len;
		newEntry.lengthWithPad = lengthWithPad;
		newEntry.level = level;

		// Copy string data

		stringDataUsed += lengthWithPad;
		std::memcpy(stringLocation, text.str, text.len);
		stringLocation[text.len] = '\0';
	}
}

void ConsoleView::AddCommandEntry(kokko::ConstStringView text)
{

}

void ConsoleView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(600, 450), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			const float reserveFooter = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -reserveFooter), false, ImGuiWindowFlags_HorizontalScrollbar);
			//if (ImGui::BeginPopupContextWindow())
			//{
			//	if (ImGui::Selectable("Clear")) ClearLog();
			//	ImGui::EndPopup();
			//}

			// Display every line as a separate entry so we can change their color or add custom widgets.
			// If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
			// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping
			// to only process visible items. The clipper will automatically measure the height of your first item and then
			// "seek" to display only items in the visible area.
			// To use the clipper we can replace your standard loop:
			//      for (int i = 0; i < Items.Size; i++)
			//   With:
			//      ImGuiListClipper clipper;
			//      clipper.Begin(Items.Size);
			//      while (clipper.Step())
			//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			// - That your items are evenly spaced (same height)
			// - That you have cheap random access to your elements (you can access them given their index,
			//   without processing all the ones before)
			// You cannot this code as-is if a filter is active because it breaks the 'cheap random-access' property.
			// We would need random-access on the post-filtered list.
			// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices
			// or offsets of items that passed the filtering test, recomputing this array when user changes the filter,
			// and appending newly elements as they are inserted. This is left as a task to the user until we can manage
			// to improve this example code!
			// If your items are of variable height:
			// - Split them into same height items would be simpler and facilitate random-seeking into your list.
			// - Consider using manual call to IsRectVisible() and skipping extraneous decoration from your items.
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

			for (int i = 0; i < entries.GetCount(); i++)
			{
				const LogEntry& entry = entries[i];
				//if (!Filter.PassFilter(item))
				//	continue;

				// Normally you would store more information in your item than just a string.
				// (e.g. make Items[] an array of structure, store color/type etc.)
				ImVec4 color;
				bool has_color = false;
				//if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
				//else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
				if (has_color)
					ImGui::PushStyleColor(ImGuiCol_Text, color);
				ImGui::TextUnformatted(entry.text);
				if (has_color)
					ImGui::PopStyleColor();
			}

			//if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			//	ImGui::SetScrollHereY(1.0f);
			//ScrollToBottom = false;

			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::Separator();

			bool reclaimFocus = false;
			auto callback = [](ImGuiInputTextCallbackData* data) {
				ConsoleView* console = static_cast<ConsoleView*>(data->UserData);
				return console->TextEditCallback(data);
			};
			ImGuiInputTextFlags input_flags = ImGuiInputTextFlags_EnterReturnsTrue |
				ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
			ImGui::PushItemWidth(-1);
			if (ImGui::InputText("##Input", inputBuffer, IM_ARRAYSIZE(inputBuffer), input_flags, callback, (void*)this))
			{
				// TODO: Execute
				//char* s = inputBuffer;
				//Strtrim(s);
				//if (s[0])
				//	ExecCommand(s);
				//strcpy(s, "");

				reclaimFocus = true;
			}
			ImGui::PopItemWidth();

			// Auto-focus on window apparition
			ImGui::SetItemDefaultFocus();
			if (reclaimFocus)
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

int ConsoleView::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	/*
	//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Example of TEXT COMPLETION

		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build a list of candidates
		ImVector<const char*> candidates;
		for (int i = 0; i < Commands.Size; i++)
			if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
				candidates.push_back(Commands[i]);

		if (candidates.Size == 0)
		{
			// No match
			AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Size == 1)
		{
			// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0]);
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches. Complete as much as we can..
			// So inputing "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c == 0 || c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
			}

			// List matches
			AddLog("Possible matches:\n");
			for (int i = 0; i < candidates.Size; i++)
				AddLog("- %s\n", candidates[i]);
		}

		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		// Example of HISTORY
		const int prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Size - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Size)
					HistoryPos = -1;
		}

		// A better implementation would preserve the data on the current input line along with cursor position.
		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
	}
	}
	*/
	return 0;
}

/*

void ConsoleView::UpdateAndDraw()
{
	//InputView* input = window->GetInputManager()->GetGameInputView();
    InputView* input = nullptr;
    if (input == nullptr)
        return;

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
	*/

} // namespace editor
} // namespace kokko
