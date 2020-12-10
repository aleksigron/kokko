#pragma once

#include "Core/Array.hpp"
#include "Core/StringRef.hpp"

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

	void Log(const char* text, LogLevel level = LogLevel::Info);
	void Log(const String& text, LogLevel level = LogLevel::Info);
	void Log(StringRef text, LogLevel level = LogLevel::Info);

	void FlushFileWrites();
};
