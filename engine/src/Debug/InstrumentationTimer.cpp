#include "Debug/InstrumentationTimer.hpp"

#include <thread>

#include "Debug/Instrumentation.hpp"

InstrumentationTimer::InstrumentationTimer(const char* name)
	: name(name), stopped(false)
{
	startTime = std::chrono::high_resolution_clock::now();
}

InstrumentationTimer::~InstrumentationTimer()
{
	if (!stopped)
		Stop();
}

void InstrumentationTimer::Stop()
{
	using namespace std::chrono;

	time_point<high_resolution_clock> endTimepoint = high_resolution_clock::now();

	long long startNano = time_point_cast<nanoseconds>(startTime).time_since_epoch().count();
	long long endNano = time_point_cast<nanoseconds>(endTimepoint).time_since_epoch().count();

	double startMicro = startNano / 1000.0;
	double endMicro = endNano / 1000.0;

	size_t thread = std::hash<std::thread::id>()(std::this_thread::get_id());
	Instrumentation::Get().WriteProfile(name, startMicro, endMicro, thread);

	stopped = true;
}

