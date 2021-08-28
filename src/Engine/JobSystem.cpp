#include "Engine/JobSystem.hpp"

#include <cassert>
#include <cstdint>
#include <new>

#include "doctest/doctest.h"

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobWorker.hpp"

#include "Memory/Allocator.hpp"

JobSystem::JobSystem(Allocator* allocator, int numWorkers) :
	allocator(allocator),
	workerCount(numWorkers),
	workers(nullptr),
	queue(allocator),
	jobsToDelete(nullptr),
	jobsToDeleteCount(0)
{
	assert(workerCount > 0);
}

JobSystem::~JobSystem()
{
	Deinitialize();
}

void JobSystem::Initialize()
{
	if (workers == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(JobWorker) * workerCount, KK_CACHE_LINE);
		workers = static_cast<JobWorker*>(buf);

		for (int i = 0; i < workerCount; ++i)
			new (&workers[i]) JobWorker(this);

		for (int i = 0; i < workerCount; ++i)
			workers[i].StartThread();
	}

	if (jobsToDelete == nullptr)
	{
		void* buf = allocator->Allocate(sizeof(Job*) * MaxJobsToDelete);
		jobsToDelete = static_cast<Job**>(buf);
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

	if (jobsToDelete != nullptr)
	{
		allocator->Deallocate(workers);
		jobsToDelete = nullptr;
	}
}

Job* JobSystem::CreateJob(JobFunction function, void* userData)
{
	Job* job = AllocateJob();
	job->function = function;
	job->userData = userData;
	job->unfinishedJobs = 1;

	return job;
}

/*
Job* JobSystem::CreateJobAsChild(JobFunction function, void* userData, Job* parent)
{
	parent->unfinishedJobs.fetch_add(1);

	Job* job = AllocateJob();
	job->function = function;
	job->userData = userData;
	job->parent = parent;
	job->unfinishedJobs = 1;

	return job;
}
*/

void JobSystem::Enqueue(size_t count, Job** jobs)
{
	assert(count > 0);

	{
		std::lock_guard<std::mutex> lock(queueMutex);
		queue.Push(jobs, count);
	}

	if (count == 1)
		jobAddedCondition.notify_one();
	else
		jobAddedCondition.notify_all();
}

void JobSystem::Wait(const Job* job)
{
	// Wait until the job has completed
	while (HasJobCompleted(job) == false)
	{
		// While waiting, work on any other job
		Job* nextJob;

		{
			std::lock_guard<std::mutex> lock(queueMutex);
			nextJob = GetJobToExecute();
		}

		if (nextJob != nullptr)
		{
			Execute(nextJob);
		}
	}
}

void JobSystem::ReleaseCompletedJobs()
{
	size_t deleteCount = jobsToDeleteCount.exchange(0);

	for (size_t i = 0; i < deleteCount; ++i)
	{
		ReleaseJob(jobsToDelete[i]);
		jobsToDelete[i] = nullptr;
	}
}

bool JobSystem::HasJobCompleted(const Job* job)
{
	return job->unfinishedJobs.load() < 0;
}

Job* JobSystem::GetJobToExecute()
{
	Job* job = nullptr;
	queue.TryPop(job);
	return job;
}

void JobSystem::Execute(Job* job)
{
	job->function(job->userData);
	Finish(job);
}

void JobSystem::Finish(Job* job)
{
	const int unfinishedJobs = job->unfinishedJobs.fetch_sub(1);

	// fetch_sub returns the previous value, so 1 means the value now is 0
	if (unfinishedJobs == 1)
	{
		const size_t index = jobsToDeleteCount.fetch_add(1);

		// Make sure we never get close to filling the delete array
		assert(index < (MaxJobsToDelete / 2));

		jobsToDelete[index] = job;

		// No child job support yet
		//if (job->parent != nullptr)
		//{
		//	Finish(job->parent);
		//}

		// Decrement unfinishedJobs to -1 to mark it as safe to delete
		job->unfinishedJobs.fetch_sub(1);
	}
}

Job* JobSystem::AllocateJob()
{
	return allocator->MakeNew<Job>();
}

void JobSystem::ReleaseJob(Job* job)
{
	allocator->MakeDelete(job);
}

struct TestFnData
{
	int64_t result;
	int64_t padding[7];
};

static void TestFn(void* data)
{
	KOKKO_PROFILE_SCOPE("TestJob");

	constexpr int64_t SumCount = 1 << 24;

	int64_t sum = 0;
	for (int64_t i = 0; i < SumCount; ++i)
		sum += i;

	static_cast<TestFnData*>(data)->result = sum;
};

TEST_CASE("JobSystem")
{
	constexpr size_t IterationCount = 5;
	constexpr size_t JobCount = 100;

	TestFnData validationResult{};
	TestFn(&validationResult);

	Allocator* allocator = Allocator::GetDefault();

	Job** jobs = static_cast<Job**>(allocator->Allocate(sizeof(Job*) * JobCount));

	void* resultsBuffer = allocator->AllocateAligned(sizeof(TestFnData) * JobCount, KK_CACHE_LINE);
	TestFnData* results = static_cast<TestFnData*>(resultsBuffer);

	for (size_t iter = 0; iter < IterationCount; ++iter)
	{
		std::memset(jobs, 0, sizeof(Job*) * JobCount);
		std::memset(results, 0, sizeof(TestFnData) * JobCount);

		JobSystem jobSystem(allocator, 3);
		jobSystem.Initialize();

		for (size_t j = 0; j < JobCount; ++j)
			jobs[j] = jobSystem.CreateJob(TestFn, &results[j]);

		jobSystem.Enqueue(JobCount, jobs);

		for (size_t j = 0; j < JobCount; ++j)
			jobSystem.Wait(jobs[j]);

		jobSystem.ReleaseCompletedJobs();

		jobSystem.Deinitialize();

		for (int j = 0; j < JobCount; ++j)
			CHECK(results[j].result == validationResult.result);
	}

	allocator->Deallocate(resultsBuffer);
	allocator->Deallocate(jobs);
}
