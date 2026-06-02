#pragma once

#define KOKKO_ARRAY_ITEMS(x) (sizeof(x) / sizeof(x[0]))

#ifdef _MSC_VER
#define KOKKO_FUNC_SIG __FUNCSIG__
#else
#define KOKKO_FUNC_SIG __PRETTY_FUNCTION__
#endif

#if TRACY_ENABLE
#include <stdio.h>
#include "tracy/Tracy.hpp"
#define KOKKO_PROFILE_FUNCTION() ZoneScoped
#define KOKKO_PROFILE_SCOPE(name) ZoneScopedN(name)
#define KOKKO_PROFILE_ADDTEXT(fmt, ...) \
    do { \
        char tracy_buf[256]; \
        int tracy_written = snprintf(tracy_buf, sizeof(tracy_buf), fmt, ##__VA_ARGS__); \
        if (tracy_written > 0) { \
            size_t tracy_len = (static_cast<size_t>(tracy_written) < sizeof(tracy_buf)) \
                            ? static_cast<size_t>(tracy_written) : (sizeof(tracy_buf) - 1); \
            TracyMessage(tracy_buf, tracy_len); \
        } \
    } while(false)
#define KOKKO_FRAME_MARK() FrameMark
#else
#define KOKKO_PROFILE_FUNCTION()
#define KOKKO_PROFILE_SCOPE(name)
#define KOKKO_PROFILE_ADDTEXT(fmt, ...)
#define KOKKO_FRAME_MARK()
#endif

#include "System/Log.hpp"

#define KK_LOG_DEBUG(format, ...) ::kokko::Log::LogDebug(__FILE__, __LINE__, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_INFO(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Info, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_WARN(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Warning, FMT_STRING(format), ## __VA_ARGS__)
#define KK_LOG_ERROR(format, ...) ::kokko::Log::Log(::kokko::LogLevel::Error, FMT_STRING(format), ## __VA_ARGS__)

#define KK_CACHE_LINE 64
