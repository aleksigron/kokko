#pragma once

#include <chrono>

class PerformanceTimer
{
private:
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;

public:
	PerformanceTimer() : startTime(std::chrono::high_resolution_clock::now())
	{
	}

	void Restart()
	{
		startTime = std::chrono::high_resolution_clock::now();
	}

	std::chrono::high_resolution_clock::duration GetDurationFromStartToNow() const
	{
		return std::chrono::high_resolution_clock::now() - startTime;
	}

	double ElapsedSeconds() const
	{
		return ElapsedNanoseconds() * (std::chrono::nanoseconds::period::num / double(std::chrono::nanoseconds::period::den));
	}

	long long ElapsedNanoseconds() const
	{
		return std::chrono::duration_cast<std::chrono::nanoseconds>(GetDurationFromStartToNow()).count();
	}
};
