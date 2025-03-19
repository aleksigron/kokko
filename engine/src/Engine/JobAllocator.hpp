#pragma once

#include <atomic>

#include "Core/Core.hpp"

#include "Engine/JobSystem.hpp"

namespace kokko
{

class Allocator;

struct Job;

class JobAllocator
{
public:
	JobAllocator(Allocator* allocator);
	~JobAllocator();

	Job* AllocateJob();

	void Reset();

private:
	Allocator* allocator;
	Job* jobs;
	std::atomic_size_t jobIndex;

	static const size_t CL = KK_CACHE_LINE;
	static const size_t MemberBytes = sizeof(Allocator*) + sizeof(Job*) + sizeof(std::atomic_size_t);
	uint8_t padding[(MemberBytes + CL - 1) / CL * CL - MemberBytes];
};

} // namespace kokko
