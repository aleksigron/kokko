#pragma once

#include "Core/Array.hpp"

#include "System/Logger.hpp"

class Allocator;

namespace kokko
{
namespace editor
{

class ConsoleView;

class ConsoleLogger : public kokko::Logger::Receiver
{
public:
	ConsoleLogger() = delete;
	explicit ConsoleLogger(Allocator* allocator);
	ConsoleLogger(const ConsoleLogger&) = delete;
	ConsoleLogger(ConsoleLogger&&) = delete;

	virtual void Log(const char* text, size_t length, LogLevel level) override;

	void SetConsoleView(ConsoleView* console);

private:
	struct Entry
	{
		size_t offset;
		size_t length;
		LogLevel level;
	};

	ConsoleView* consoleView;
	Array<Entry> bufferedEntries;
	Array<char> bufferedStringData;
};

} // namespace editor
} // namespace kokko
