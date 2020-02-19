#include "Debug/LogHelper.hpp"

#include "Core/StringRef.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"

static DebugLog* debugLog = nullptr;

void Log::SetLogInstance(DebugLog* instance)
{
	debugLog = instance;
}

void Log::Info(const char* str, unsigned int len)
{
	debugLog->Log(StringRef(str, len));
}

void Log::Info(const char* str)
{
	debugLog->Log(str);
}
