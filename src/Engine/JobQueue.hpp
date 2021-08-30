#pragma once

#include <cstdint>
#include <mutex>

#include "Core/Core.hpp"

class Allocator;

struct Job;

class JobQueue
{
public:
	JobQueue(Allocator* allocator);
	~JobQueue();

	void Push(Job* job);
	Job* Pop();
	Job* Steal();

private:
	static const size_t MaxJobCount = 1 << 12;
	static const size_t JobIndexMask = MaxJobCount - 1;

	Allocator* allocator;
	Job** jobs;
	int top;
	int bottom;

	std::mutex mutex;

	static const size_t CL = KK_CACHE_LINE;
	static const size_t MemberBytes =
		sizeof(Allocator*) + sizeof(Job**) + sizeof(size_t) * 2 + sizeof(std::mutex);
	uint8_t padding[(MemberBytes + CL - 1) / CL * CL - MemberBytes];
};
