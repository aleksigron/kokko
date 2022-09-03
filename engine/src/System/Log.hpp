#pragma once

#include <fmt/format.h>

#include "System/LogLevel.hpp"

namespace kokko
{
class Logger;

namespace Log
{
	void SetLogInstance(kokko::Logger* instance);

	void _DebugVarLog(const char* file, int line, fmt::string_view format, fmt::format_args args);
	void _VarLog(LogLevel level, fmt::string_view format, fmt::format_args args);

	template <typename S, typename... Args>
	void LogDebug(const char* file, int line, const S& format, Args&&... args)
	{
		_DebugVarLog(file, line, format, fmt::make_args_checked<Args...>(format, args...));
	}

	template <typename S, typename... Args>
	void Log(LogLevel level, const S& format, Args&&... args)
	{
		_VarLog(level, format, fmt::make_args_checked<Args...>(format, args...));
	}
}

}
