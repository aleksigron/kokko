#pragma once

#include <thread>
#include <cstdint>

class JobSystem;

class JobWorker
{
public:
	JobWorker();
	JobWorker(JobSystem* jobSystem);

	void StartThread();

	void RequestExit();
	void WaitToExit();

private:
	std::thread thread;
	JobSystem* jobSystem;
	bool exitRequested;

	// Set padding size to align the class size to a cache line to avoid false sharing
	static const size_t CacheLine = 64;
	static const size_t MemberBytes = sizeof(std::thread) + sizeof(JobSystem*) + sizeof(bool);
	uint8_t padding[(MemberBytes + CacheLine - 1) / CacheLine * CacheLine - MemberBytes];

	static void ThreadFunc(JobWorker* self);
};
