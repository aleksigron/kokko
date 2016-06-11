#include "DebugLog.hpp"

#include <cstdio>

DebugLog::DebugLog() : fileHandle(nullptr)
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

void DebugLog::Log(StringRef text)
{
	FILE* file = static_cast<FILE*>(fileHandle);

	if (fileHandle != nullptr)
	{
		// Write to log file
		std::fwrite(text.str, 1, text.len, file);
		std::fwrite("\n", 1, 1, file);
	}

	// Write to standard output (console)
	std::fwrite(text.str, 1, text.len, stdout);
	std::fwrite("\n", 1, 1, stdout);
}

void DebugLog::FlushFileWrites()
{
	FILE* file = static_cast<FILE*>(fileHandle);

	if (fileHandle != nullptr)
	{
		std::fflush(file);
	}
}
