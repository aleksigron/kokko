#pragma once

#include "Core/Queue.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "System/LogLevel.hpp"

#include "EditorWindow.hpp"

struct ImGuiInputTextCallbackData;

class Allocator;

namespace kokko
{

namespace editor
{

class ConsoleView : public EditorWindow
{
private:
	struct LogEntry
	{
		const char* text;
		uint32_t length;
		uint32_t lengthWithPad;
		LogLevel level;
	};

	Allocator* allocator;

	Queue<LogEntry> entries;

	char* stringData;
	size_t stringDataFirst;
	size_t stringDataUsed;
	size_t stringDataAllocated;

	unsigned int totalWarningCount;
	unsigned int totalErrorCount;

	char inputBuffer[2048];

	int TextEditCallback(ImGuiInputTextCallbackData* data);

public:
	ConsoleView(Allocator* allocator);
	~ConsoleView();

	virtual void Update(EditorContext& context) override;

	unsigned int GetTotalWarningCount() const { return totalWarningCount; }
	unsigned int GetTotalErrorCount() const { return totalErrorCount; }

	void AddLogEntry(kokko::ConstStringView text, LogLevel level = LogLevel::Info);
	void AddCommandEntry(kokko::ConstStringView text);
};

} // namespace editor
} // namespace kokko
