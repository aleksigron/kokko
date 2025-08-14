#include "Engine/JobQueue.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

#include "Memory/Allocator.hpp"

// Lock free implementation is based on post on Molecular Musings blog
// https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/

namespace kokko
{

JobQueue::JobQueue(Allocator* allocator) :
	allocator(allocator),
	jobs(nullptr),
	top(0),
	bottom(0)
{
	std::memset(padding, 0, sizeof(padding));
	jobs = static_cast<Job**>(allocator->Allocate(MaxJobCount * sizeof(Job), "JobQueue.jobs"));
}

JobQueue::~JobQueue()
{
	allocator->Deallocate(jobs);
}

void JobQueue::Push(Job* job)
{
	int64_t b = bottom.load(std::memory_order_relaxed);
	jobs[b & JobIndexMask] = job;

	// Atomic store ensures the job is written before b+1 is published to other threads
	// Release semantics prevent memory reordering of operations preceding this
	bottom.store(b + 1, std::memory_order_release);
}

Job* JobQueue::Pop()
{
	// Read-modify-write to bottom must happen before the load from top
	// This is accomplished with acquire ordering
	int64_t b = bottom.fetch_sub(1, std::memory_order_acquire) - 1;

	int64_t t = top.load(std::memory_order_relaxed);
	if (t <= b)
	{
		// Queue is not empty since t<=b
		Job* job = jobs[b & JobIndexMask];
		if (t != b)
		{
			// There's still more than one item left in the queue
			return job;
		}

		// This is the last item in the queue
		// Original code uses Windows intrinsic _InterlockedCompareExchange
		// Unlike _InterlockedCompareExchange, compare_exchange_strong changes expected, so we create a copy here
		// _InterlockedCompareExchange serves as a full memory barrier, so we need at least acq_rel memory ordering
		int64_t expectedT = t;
		if (top.compare_exchange_strong(expectedT, t + 1, std::memory_order_acq_rel) == false)
		{
			// A concurrent steal operation removed an element from the queue
			job = nullptr;
		}

		bottom.store(t + 1, std::memory_order_relaxed);
		return job;
	}
	else
	{
		// Queue was already empty
		// Reset bottom to be same as top loaded
		bottom.store(t, std::memory_order_relaxed);
		return nullptr;
	}
}

Job* JobQueue::Steal()
{
	// Atomic load with acquire semantics ensures that top is always read before bottom
	int64_t t = top.load(std::memory_order_acquire);

	// We can use relaxed ordering for bottom, since executing the next instructions is
	// dependent on the value of bottom
	int64_t b = bottom.load(std::memory_order_relaxed);

	if (t < b) // Queue is not empty
	{
		Job* job = jobs[t & JobIndexMask];

		// Compare-exchange operation with release ordering guarantees that
		// the jobs array read happens before it
		if (top.compare_exchange_strong(t, t + 1, std::memory_order_release) == false)
		{
			// If compare_exchange returns false,
			// a concurrent steal or pop operation removed an element from the queue
			return nullptr;
		}

		return job;
	}
	else
	{
		// Queue was empty
		return nullptr;
	}
}

bool JobQueue::HasWork() const
{
	return top.load() != bottom.load();
}

} // namespace kokko
