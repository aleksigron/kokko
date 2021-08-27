#include "Engine/JobSystem.hpp"

#include <cassert>
#include <cstdint>

#include "doctest/doctest.h"

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobWorker.hpp"

#include "Memory/Allocator.hpp"

JobSystem::JobSystem(Allocator* allocator, int numWorkers) :
	allocator(allocator),
	queue(allocator),
	workers(nullptr),
	workerCount(numWorkers)
{
	assert(workerCount > 0);
}

JobSystem::~JobSystem()
{
}

void JobSystem::Initialize()
{
	if (workers == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(JobWorker) * workerCount, KK_CACHE_LINE);
		workers = static_cast<JobWorker*>(buf);

		for (int i = 0; i < workerCount; ++i)
			workers[i] = JobWorker(this);

		for (int i = 0; i < workerCount; ++i)
			workers[i].StartThread();
	}
}

void JobSystem::Deinitialize()
{
	if (workers != nullptr)
	{
		for (int i = 0; i < workerCount; ++i)
			workers[i].RequestExit();

		// Lock must be acquired before notifying the condition
		std::unique_lock<std::mutex> lock(queueMutex);
		lock.unlock();

		jobAddedCondition.notify_all();

		for (int i = 0; i < workerCount; ++i)
			workers[i].WaitToExit();

		allocator->Deallocate(workers);
		workers = nullptr;
	}
}

void JobSystem::AddJob(JobFunction function, void* userData)
{
	Job* job = allocator->MakeNew<Job>();
	job->function = function;
	job->userData = userData;
	job->unfinishedJobs = 1;

	{
		std::lock_guard<std::mutex> lock{ queueMutex };
		queue.Push(job);
	}

	jobAddedCondition.notify_one();
}
/*
void Wait(const Job* job)
{
	// wait until the job has completed. in the meantime, work on any other job.
	while (!HasJobCompleted(job))
	{
		Job* nextJob = GetJob();
		if (nextJob)
		{
			Execute(nextJob);
		}
	}
}
*/
static void TestFn(void* data)
{
	KOKKO_PROFILE_SCOPE("TestJob");

	constexpr int64_t SumCount = 1 << 24;

	int64_t sum = 0;
	for (int64_t i = 0; i < SumCount; ++i)
		sum += i;

	int64_t* result = static_cast<int64_t*>(data);
	*result = sum;
};

TEST_CASE("JobSystem")
{
	constexpr size_t IterationCount = 100;

	int64_t validationResult = 0;
	TestFn(&validationResult);

	Allocator* allocator = Allocator::GetDefault();
	JobSystem jobSystem(allocator, 2);
	jobSystem.Initialize();

	int64_t* results = static_cast<int64_t*>(allocator->Allocate(sizeof(int64_t) * IterationCount));

	for (int i = 0; i < IterationCount; ++i)
		jobSystem.AddJob(TestFn, &results[i]);
	
	// This will currently wait for the jobs to finish
	jobSystem.Deinitialize();

	for (int i = 0; i < IterationCount; ++i)
	{
		CHECK(results[i] == validationResult);
	}

	allocator->Deallocate(results);
}
