#include "System/Logger.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/DebugConsole.hpp"

namespace kokko
{

Logger::Logger(Allocator* allocator) :
	fileHandle(nullptr),
	formatBuffer(allocator)
{
}

Logger::~Logger()
{
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);
		std::fclose(file);
	}
}

bool Logger::OpenLogFile(const char* filePath, bool append)
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

	KK_LOG_ERROR("Debug log file couldn't be opened for writing");
	return false;
}

void Logger::Log(const char* text, size_t length, LogLevel level)
{
    // TODO: Reimplement debug console

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

}