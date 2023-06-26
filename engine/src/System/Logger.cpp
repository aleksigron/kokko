#include "System/Logger.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

namespace kokko
{

Logger::Logger(Allocator* allocator) :
	fileHandle(nullptr),
	receiver(nullptr),
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

void Logger::SetReceiver(Receiver* receiver)
{
	this->receiver = receiver;
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
	if (fileHandle != nullptr)
	{
		FILE* file = static_cast<FILE*>(fileHandle);

		// Write to log file
		std::fwrite(text, 1, length, file);
		std::fputc('\n', file);
	}

	if (receiver != nullptr)
	{
		receiver->Log(text, length, level);
	}

	// Write to standard output (console)
	std::fwrite(text, 1, length, stdout);
	std::fputc('\n', stdout);
}

}
