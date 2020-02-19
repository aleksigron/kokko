#pragma once

class DebugLog;

namespace Log
{
	enum class Level
	{
		Verbose,
		Info,
		Warning,
		Error
	};

	void SetLogInstance(DebugLog* instance);

	void Info(const char* str, unsigned int len);
	void Info(const char* str);
}
