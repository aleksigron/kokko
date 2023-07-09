#include "System/ConsoleLogger.hpp"

#include "Views/ConsoleView.hpp"

namespace kokko
{
namespace editor
{

ConsoleLogger::ConsoleLogger(Allocator* allocator) :
	consoleView(nullptr),
	bufferedEntries(allocator),
	bufferedStringData(allocator)
{
}

void ConsoleLogger::Log(const char* text, size_t length, LogLevel level)
{
	if (consoleView != nullptr)
	{
		consoleView->AddLogEntry(ConstStringView(text, length), level);
	}
	else
	{
		// Save log to be sent to console later

		size_t offset = bufferedStringData.GetCount();
		bufferedStringData.Resize(offset + length);
		std::memcpy(bufferedStringData.GetData() + offset, text, length);

		bufferedEntries.PushBack(Entry{ offset, length, level });
	}
}

void ConsoleLogger::SetConsoleView(ConsoleView* console)
{
	consoleView = console;

	if (consoleView != nullptr && bufferedEntries.GetCount() != 0)
	{
		// Send buffered logs to console

		const char* const strData = bufferedStringData.GetData();
		for (const auto& entry : bufferedEntries)
		{
			consoleView->AddLogEntry(ConstStringView(strData + entry.offset, entry.length), entry.level);
		}

		bufferedEntries.ClearAndRelease();
		bufferedStringData.ClearAndRelease();
	}
}

} // namespace editor
} // namespace kokko
