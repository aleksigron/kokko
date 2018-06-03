#pragma once

#include "StringRef.hpp"

class DebugConsole;

class DebugLog
{
private:
	void* fileHandle;

	DebugConsole* console;

public:
	DebugLog(DebugConsole* console);
	~DebugLog();

	bool OpenLogFile(const char* filePath, bool append);

	void Log(StringRef text);

	void FlushFileWrites();
};
