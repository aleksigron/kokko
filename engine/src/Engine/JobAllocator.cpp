#include "Engine/JobAllocator.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

#include "Memory/Allocator.hpp"

JobAllocator::JobAllocator(Allocator* allocator) :
	allocator(allocator),
	jobs(nullptr),
	jobIndex(0)
{
	const size_t maxJobs = JobSystem::MaxJobsPerThreadPerFrame;
	void* buf = allocator->AllocateAligned(sizeof(Job) * maxJobs, KK_CACHE_LINE);
	jobs = static_cast<Job*>(buf);

	std::memset(padding, 0, sizeof(padding));
}

JobAllocator::~JobAllocator()
{
	allocator->Deallocate(jobs);
}

Job* JobAllocator::AllocateJob()
{
	size_t index = jobIndex.fetch_add(1);

	assert(index < JobSystem::MaxJobsPerThreadPerFrame);

	return &jobs[index];
}

void JobAllocator::Reset()
{
	assert(jobIndex < JobSystem::MaxJobsPerThreadPerFrame);
	jobIndex.store(0);
}
