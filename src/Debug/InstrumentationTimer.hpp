#pragma once

#include <chrono>

#include "Debug/Instrumentation.hpp"

class InstrumentationTimer
{
private:
	const char* name;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	bool stopped;

public:
	InstrumentationTimer(const char* name)
		: name(name), stopped(false)
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	~InstrumentationTimer()
	{
		if (!stopped)
			Stop();
	}

	void Stop()
	{
		using namespace std::chrono;

		time_point<high_resolution_clock> endTimepoint = high_resolution_clock::now();

		long long startNano = time_point_cast<nanoseconds>(startTime).time_since_epoch().count();
		long long endNano = time_point_cast<nanoseconds>(endTimepoint).time_since_epoch().count();

		double startMicro = startNano / 1000.0;
		double endMicro = endNano / 1000.0;

		Instrumentation::Get().WriteProfile(name, startMicro, endMicro, 0);

		stopped = true;
	}
};
