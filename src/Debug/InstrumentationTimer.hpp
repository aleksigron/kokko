#pragma once

#include <chrono>

class InstrumentationTimer
{
private:
	const char* name;
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
	bool stopped;

public:
	InstrumentationTimer(const char* name);
	~InstrumentationTimer();

	void Stop();
};
