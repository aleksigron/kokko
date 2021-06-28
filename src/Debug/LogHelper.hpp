#pragma once

#include <fmt/format.h>

#include "Debug/LogLevel.hpp"

#define KK_LOG_DEBUG(format, ...) ::Log::LogDebug(__FILE__, __LINE__, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_INFO(format, ...) ::Log::Log(LogLevel::Info, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_WARN(format, ...) ::Log::Log(LogLevel::Warning, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_ERROR(format, ...) ::Log::Log(LogLevel::Error, FMT_STRING(format), __VA_ARGS__)

class DebugLog;

namespace Log
{
	void SetLogInstance(DebugLog* instance);

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
