#pragma once

#include "Core/Array.hpp"
#include "Core/StringView.hpp"

#include "Debug/LogLevel.hpp"

class Allocator;
class String;
class DebugConsole;

class DebugLog
{
private:
	void* fileHandle;

	DebugConsole* console;

	Array<char> formatBuffer;

public:
	DebugLog(Allocator* allocator, DebugConsole* console);
	~DebugLog();

	bool OpenLogFile(const char* filePath, bool append);

	void Log(const char* text, size_t length, LogLevel level);

	Array<char>& GetFormatBuffer() { return formatBuffer; }
};
