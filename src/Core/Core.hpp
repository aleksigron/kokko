#pragma once

#define KK_UNIQUE_NAME_(name, counter) name##counter
#define KK_UNIQUE_NAME(name, counter) KK_UNIQUE_NAME_(name, counter)

#define KOKKO_PROFILING_ENABLED 1

#if KOKKO_PROFILING_ENABLED
#include "Debug/InstrumentationTimer.hpp"
#define KOKKO_PROFILE_SCOPE(name) InstrumentationTimer KK_UNIQUE_NAME(instrTimer, __LINE__)(name)
#define KOKKO_PROFILE_FUNCTION() KOKKO_PROFILE_SCOPE(__FUNCSIG__)
#else
#define KOKKO_PROFILE_SCOPE(name)
#define KOKKO_PROFILE_FUNCTION()
#endif

#include "Debug/Log.hpp"

#define KK_LOG_DEBUG(format, ...) ::Log::LogDebug(__FILE__, __LINE__, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_INFO(format, ...) ::Log::Log(LogLevel::Info, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_WARN(format, ...) ::Log::Log(LogLevel::Warning, FMT_STRING(format), __VA_ARGS__)
#define KK_LOG_ERROR(format, ...) ::Log::Log(LogLevel::Error, FMT_STRING(format), __VA_ARGS__)

#define KK_CACHE_LINE 64
