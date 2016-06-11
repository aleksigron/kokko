#pragma once

#include "StringRef.hpp"

class DebugLog
{
private:
	void* fileHandle;

public:
	DebugLog();
	~DebugLog();

	bool OpenLogFile(const char* filePath, bool append);

	void Log(StringRef text);

	void FlushFileWrites();
};
