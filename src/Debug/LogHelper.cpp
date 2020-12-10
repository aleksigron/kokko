#include "Debug/LogHelper.hpp"

#include "Core/StringRef.hpp"

#include "Debug/DebugLog.hpp"
#include "Debug/LogLevel.hpp"

static DebugLog* debugLog = nullptr;

void Log::SetLogInstance(DebugLog* instance)
{
	debugLog = instance;
}

void Log::Info(const char* str, unsigned int len)
{
	debugLog->Log(StringRef(str, len), LogLevel::Info);
}

void Log::Info(const char* str)
{
	debugLog->Log(str, LogLevel::Info);
}

void Log::Warning(const char* str, unsigned int len)
{
	debugLog->Log(StringRef(str, len), LogLevel::Warning);
}

void Log::Warning(const char* str)
{
	debugLog->Log(str, LogLevel::Warning);
}

void Log::Error(const char* str, unsigned int len)
{
	debugLog->Log(StringRef(str, len), LogLevel::Error);
}

void Log::Error(const char* str)
{
	debugLog->Log(str, LogLevel::Error);
}
