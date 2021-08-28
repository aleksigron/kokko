#pragma once

#include <atomic>
#include <cstdint>
#include <thread>

class JobSystem;

class JobWorker
{
public:
	explicit JobWorker(JobSystem* jobSystem);

	void StartThread();

	void RequestExit();
	void WaitToExit();

private:
	std::thread thread;
	JobSystem* jobSystem;
	std::atomic_bool exitRequested;

	// Set padding size to align the class size to a cache line to avoid false sharing
	static const size_t CacheLine = 64;
	static const size_t MemberBytes = sizeof(std::thread) + sizeof(JobSystem*) + sizeof(std::atomic_bool);
	uint8_t padding[(MemberBytes + CacheLine - 1) / CacheLine * CacheLine - MemberBytes];

	void ThreadMain();
};
