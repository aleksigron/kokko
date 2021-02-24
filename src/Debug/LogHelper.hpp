#pragma once

#include "Debug/LogLevel.hpp"

class DebugLog;

namespace Log
{
	void SetLogInstance(DebugLog* instance);
	
	void Log(LogLevel level, const char* str, unsigned int len);
	void Log(LogLevel level, const char* str);

	void Info(const char* str, unsigned int len);
	void Info(const char* str);

	void Warning(const char* str, unsigned int len);
	void Warning(const char* str);

	void Error(const char* str, unsigned int len);
	void Error(const char* str);
}
