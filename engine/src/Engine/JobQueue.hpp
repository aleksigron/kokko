#pragma once

#include <cstddef>
#include <atomic>

#include "Core/Core.hpp"

namespace kokko
{

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
	std::atomic_int64_t top;
	std::atomic_int64_t bottom;

	static const size_t CL = KK_CACHE_LINE;
	static const size_t MemberBytes = sizeof(Allocator*) + sizeof(Job**) + sizeof(long) * 2;
	uint8_t padding[(MemberBytes + CL - 1) / CL * CL - MemberBytes];
};

} // namespace kokko
