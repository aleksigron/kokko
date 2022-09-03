#pragma once

#include "Core/Array.hpp"

#include "System/LogLevel.hpp"

class Allocator;

namespace kokko
{

class Logger
{
private:
	void* fileHandle;

	Array<char> formatBuffer;

public:
    Logger(Allocator* allocator);
	~Logger();

	bool OpenLogFile(const char* filePath, bool append);

	void Log(const char* text, size_t length, LogLevel level);

	Array<char>& GetFormatBuffer() { return formatBuffer; }
};

}
