#include "Engine/JobSystem.hpp"

#include <cassert>
#include <cstdint>

#include "doctest/doctest.h"

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"

JobSystem::JobSystem(Allocator* allocator, int numWorkers) :
	queue(allocator),
	workers(allocator),
	workerCount(numWorkers)
{
	assert(workerCount > 0);
}

JobSystem::~JobSystem()
{
}

void JobSystem::Initialize()
{
	if (workers.GetCount() == 0)
	{
		workers.Resize(workerCount);

		for (int i = 0; i < workerCount; ++i)
			workers[i] = JobWorker(this);

		for (int i = 0; i < workerCount; ++i)
			workers[i].StartThread();
	}
}

void JobSystem::Deinitialize()
{
	if (workers.GetCount() > 0)
	{
		for (int i = 0; i < workerCount; ++i)
			workers[i].RequestExit();

		for (int i = 0; i < workerCount; ++i)
			workers[i].WaitToExit();

		workers.Clear();
	}
}

void JobSystem::AddJob(const Job& job)
{
	{
		std::lock_guard<std::mutex> lock{ queueMutex };
		queue.Push(job);
	}

	jobAddedCondition.notify_one();
}

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
	JobSystem jobSystem(allocator, 1);
	jobSystem.Initialize();

	int64_t* results = static_cast<int64_t*>(allocator->Allocate(sizeof(int64_t) * IterationCount));

	for (int i = 0; i < IterationCount; ++i)
	{
		Job job{ TestFn, &results[i] };
		jobSystem.AddJob(job);
	}
	
	// This will currently wait for the jobs to finish
	jobSystem.Deinitialize();

	for (int i = 0; i < IterationCount; ++i)
	{
		CHECK(results[i] == validationResult);
	}

	allocator->Deallocate(results);
}
