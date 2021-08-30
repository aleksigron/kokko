#include "Engine/JobQueue.hpp"

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

#include "Memory/Allocator.hpp"

JobQueue::JobQueue(Allocator* allocator) :
	allocator(allocator),
	jobs(nullptr),
	top(0),
	bottom(0)
{
	jobs = static_cast<Job**>(allocator->Allocate(MaxJobCount * sizeof(Job)));
}

JobQueue::~JobQueue()
{
	allocator->Deallocate(jobs);
}

void JobQueue::Push(Job* job)
{
	std::lock_guard lock(mutex);

	jobs[bottom] = job;
	++bottom;
}

Job* JobQueue::Pop()
{
	std::lock_guard lock(mutex);

	const int jobCount = bottom - top;
	if (jobCount <= 0)
	{
		// no job left in the queue
		return nullptr;
	}

	--bottom;
	return jobs[bottom & JobIndexMask];
}

Job* JobQueue::Steal()
{
	std::lock_guard lock(mutex);

	const int jobCount = bottom - top;
	if (jobCount <= 0)
	{
		// no job there to steal
		return nullptr;
	}

	Job* job = jobs[top & JobIndexMask];
	++top;
	return job;
}
