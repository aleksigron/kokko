#pragma once

#define KOKKO_PROFILING_ENABLED 1

#if KOKKO_PROFILING_ENABLED
#include "Debug/InstrumentationTimer.hpp"
#define KOKKO_PROFILE_SCOPE(name) InstrumentationTimer instrTimer##__LINE__(name)
#define KOKKO_PROFILE_FUNCTION() KOKKO_PROFILE_SCOPE(__FUNCSIG__)
#else
#define KOKKO_PROFILE_FUNCTION()
#endif
