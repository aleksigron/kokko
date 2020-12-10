#include "Debug/DebugLog.hpp"

#include <cstdio>

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

void DebugLog::Log(const char* text, LogLevel level)
{
	this->Log(StringRef(text), level);
}

void DebugLog::Log(const String& text, LogLevel level)
{
	this->Log(text.GetRef(), level);
}

void DebugLog::Log(StringRef text, LogLevel level)
{
	static const size_t levelStringLength = 10;
	static const char const levelStrings[][levelStringLength] =
	{
		"[VERBOSE]",
		"[INFO   ]",
		"[WARNING]",
		"[ERROR  ]"
	};

	const char* levelStr = levelStrings[static_cast<size_t>(level)];
	formatBuffer.Resize(levelStringLength + text.len + 2);

	char* buffer = formatBuffer.GetData();
	std::memcpy(buffer, levelStr, levelStringLength - 1);
	buffer[levelStringLength - 1] = ' ';
	std::memcpy(buffer + levelStringLength, text.str, text.len);
	buffer[levelStringLength + text.len] = '\0';

	// Add to debug log view without newline
	console->AddLogEntry(StringRef(buffer, levelStringLength + text.len), level);

	// Add newline
	buffer[levelStringLength + text.len] = '\n';
	buffer[levelStringLength + text.len + 1] = '\0';

	size_t totalLength = levelStringLength + text.len + 1;

	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);

		// Write to log file
		std::fwrite(buffer, 1, totalLength, file);
	}

	// Write to standard output (console)
	std::fwrite(buffer, 1, totalLength, stdout);
}

void DebugLog::FlushFileWrites()
{
	FILE* file = static_cast<FILE*>(fileHandle);

	if (fileHandle != nullptr)
	{
		std::fflush(file);
	}
}
