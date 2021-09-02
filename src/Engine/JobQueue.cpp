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
	long b = bottom;
	jobs[b & JobIndexMask] = job;

	// Atomic store ensures the job is written before b+1 is published to other threads
	bottom.store(b + 1);
}

Job* JobQueue::Pop()
{
	long b = bottom - 1;
	bottom.exchange(b);

	long t = top;
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
		if (top.compare_exchange_strong(t, t + 1) == false)
		{
			// A concurrent steal operation removed an element from the queue
			job = nullptr;
		}

		bottom = t + 1;
		return job;
	}
	else
	{
		// Queue was already empty
		bottom = t;
		return nullptr;
	}
}

Job* JobQueue::Steal()
{
	// Atomic loads ensure that top is always read before bottom
	long t = top.load();
	long b = bottom.load();

	if (t < b)
	{
		// Queue is not empty
		Job* job = jobs[t & JobIndexMask];

		// Compare-exchange guarantees that the read happens before it
		if (top.compare_exchange_strong(t, t + 1) == false)
		{
			// A concurrent steal or pop operation removed an element from the queue
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
