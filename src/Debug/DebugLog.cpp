#include "Debug/DebugLog.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/DebugConsole.hpp"

DebugLog::DebugLog(Allocator* allocator, DebugConsole* console) :
	fileHandle(nullptr),
	console(console),
	formatBuffer(allocator)
{

}

DebugLog::~DebugLog()
{
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);
		std::fclose(file);
	}
}

bool DebugLog::OpenLogFile(const char* filePath, bool append)
{
	KOKKO_PROFILE_FUNCTION();

	if (fileHandle == nullptr)
	{
		const char* mode = append ? "ab" : "wb";

		fileHandle = std::fopen(filePath, mode);

		if (fileHandle != nullptr)
		{
			return true;
		}
	}

	return false;
}

void DebugLog::Log(const char* text, size_t length, LogLevel level)
{
	console->AddLogEntry(StringRef(text, length), level);

	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);

		// Write to log file
		std::fwrite(text, 1, length, file);
		std::fputc('\n', file);
	}

	// Write to standard output (console)
	std::fwrite(text, 1, length, stdout);
	std::fputc('\n', stdout);
}
