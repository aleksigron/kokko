#pragma once

#include "StringRef.hpp"

class DebugLogView;

class DebugLog
{
private:
	void* fileHandle;

	DebugLogView* logView;

public:
	DebugLog(DebugLogView* logView);
	~DebugLog();

	bool OpenLogFile(const char* filePath, bool append);

	void Log(StringRef text);

	void FlushFileWrites();
};
