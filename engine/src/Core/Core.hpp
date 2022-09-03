#pragma once

#define KK_UNIQUE_NAME_(name, counter) name##counter
#define KK_UNIQUE_NAME(name, counter) KK_UNIQUE_NAME_(name, counter)

#define KOKKO_PROFILING_ENABLED 1

#ifdef _MSC_VER
#define KOKKO_FUNC_SIG __FUNCSIG__
#else
#define KOKKO_FUNC_SIG __PRETTY_FUNCTION__
#endif

#if KOKKO_PROFILING_ENABLED
#include "Debug/InstrumentationTimer.hpp"
#define KOKKO_PROFILE_SCOPE(name) InstrumentationTimer KK_UNIQUE_NAME(instrTimer, __LINE__)(name)
#define KOKKO_PROFILE_FUNCTION() KOKKO_PROFILE_SCOPE(KOKKO_FUNC_SIG)
#else
#define KOKKO_PROFILE_SCOPE(name)
#define KOKKO_PROFILE_FUNCTION()
#endif

#include "System/Log.hpp"

#define KK_LOG_DEBUG(format, ...) kokko::Log::LogDebug(__FILE__, __LINE__, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_INFO(format, ...) kokko::Log::Log(LogLevel::Info, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_WARN(format, ...) kokko::Log::Log(LogLevel::Warning, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_ERROR(format, ...) kokko::Log::Log(LogLevel::Error, FMT_STRING(format), ## __VA_ARGS__)

#define KK_CACHE_LINE 64
