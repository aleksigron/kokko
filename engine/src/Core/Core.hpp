#pragma once

#define KOKKO_ARRAY_ITEMS(x) (sizeof(x) / sizeof(x[0]))

#ifdef _MSC_VER
#define KOKKO_FUNC_SIG __FUNCSIG__
#else
#define KOKKO_FUNC_SIG __PRETTY_FUNCTION__
#endif

#if TRACY_ENABLE
#include "tracy/Tracy.hpp"
#define KOKKO_PROFILE_SCOPE(name) ZoneScoped(name)
#define KOKKO_PROFILE_FUNCTION() KOKKO_PROFILE_SCOPE(KOKKO_FUNC_SIG)
#define KOKKO_FRAME_MARK() FrameMark
#else
#define KOKKO_PROFILE_SCOPE(name)
#define KOKKO_PROFILE_FUNCTION()
#define KOKKO_FRAME_MARK()
#endif

#include "System/Log.hpp"

#define KK_LOG_DEBUG(format, ...) ::kokko::Log::LogDebug(__FILE__, __LINE__, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_INFO(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Info, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_WARN(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Warning, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_ERROR(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Error, FMT_STRING(format), ## __VA_ARGS__)

#define KK_CACHE_LINE 64
