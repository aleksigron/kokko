#include "Engine/JobSystem.hpp"

#include <cassert>
#include <cstdint>
#include <new>

#include "doctest/doctest.h"

#include "Core/Core.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobAllocator.hpp"
#include "Engine/JobQueue.hpp"
#include "Engine/JobWorker.hpp"

#include "Math/Random.hpp"

#include "Memory/Allocator.hpp"

thread_local size_t JobSystem::currentThreadIndex = 0;

JobSystem::JobSystem(Allocator* allocator, size_t numWorkers) :
	allocator(allocator),
	jobAllocators(nullptr),
	jobQueues(nullptr),
	workerCount(numWorkers),
	workers(nullptr)
{
	assert(workerCount > 0);
}

JobSystem::~JobSystem()
{
	Deinitialize();
}

void JobSystem::Initialize()
{
	size_t threadCount = workerCount + 1;

	if (jobAllocators == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(JobAllocator) * threadCount, KK_CACHE_LINE);
		jobAllocators = static_cast<JobAllocator*>(buf);

		for (int i = 0; i < threadCount; ++i)
			new (&jobAllocators[i]) JobAllocator(allocator);
	}

	if (jobQueues == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(JobQueue) * threadCount, KK_CACHE_LINE);
		jobQueues = static_cast<JobQueue*>(buf);

		for (int i = 0; i < threadCount; ++i)
			new (&jobQueues[i]) JobQueue(allocator);
	}

	if (workers == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(JobWorker) * workerCount, KK_CACHE_LINE);
		workers = static_cast<JobWorker*>(buf);

		for (int i = 0; i < workerCount; ++i)
			new (&workers[i]) JobWorker(i + 1, this);

		for (int i = 0; i < workerCount; ++i)
			workers[i].StartThread();
	}
}

void JobSystem::Deinitialize()
{
	size_t threadCount = workerCount + 1;

	if (workers != nullptr)
	{
		for (int i = 0; i < workerCount; ++i)
			workers[i].RequestExit();

		// Lock must be acquired before notifying the condition
		std::unique_lock<std::mutex> lock(conditionMutex);
		lock.unlock();

		jobAddedCondition.notify_all();

		for (int i = 0; i < workerCount; ++i)
			workers[i].WaitToExit();

		allocator->Deallocate(workers);
		workers = nullptr;
	}

	if (jobQueues != nullptr)
	{
		for (int i = 0; i < threadCount; ++i)
			jobQueues[i].~JobQueue();

		allocator->Deallocate(jobQueues);
		jobQueues = nullptr;
	}

	if (jobAllocators != nullptr)
	{
		for (int i = 0; i < threadCount; ++i)
			jobAllocators[i].~JobAllocator();

		allocator->Deallocate(jobAllocators);
		jobAllocators = nullptr;
	}
}

Job* JobSystem::CreateJob(JobFunction function)
{
	return CreateJobWithData(function, nullptr, 0);
}

Job* JobSystem::CreateJobWithPtr(JobFunction function, void* ptr)
{
	return CreateJobWithData(function, &ptr, sizeof(void*));
}

Job* JobSystem::CreateJobWithData(JobFunction function, const void* data, size_t size)
{
	assert(size <= sizeof(Job::padding));

	Job* job = AllocateJob();
	job->function = function;
	job->parent = nullptr;
	job->unfinishedJobs = 1;

	if (size > 0)
	{
		std::memcpy(job->padding, data, size);
	}

	return job;
}

Job* JobSystem::CreateJobAsChild(JobFunction function, Job* parent)
{
	return CreateJobAsChildWithData(function, parent, nullptr, 0);
}

Job* JobSystem::CreateJobAsChildWithPtr(JobFunction function, Job* parent, void* ptr)
{
	return CreateJobAsChildWithData(function, parent, &ptr, sizeof(void*));
}

Job* JobSystem::CreateJobAsChildWithData(JobFunction function, Job* parent, const void* data, size_t size)
{
	parent->unfinishedJobs.fetch_add(1);

	Job* job = AllocateJob();
	job->function = function;
	job->parent = parent;
	job->unfinishedJobs = 1;

	if (size > 0)
	{
		std::memcpy(job->padding, data, size);
	}

	return job;
}

void JobSystem::Enqueue(Job* job)
{
	JobQueue* queue = GetCurrentThreadJobQueue();

	queue->Push(job);

	jobAddedCondition.notify_all();
}

void JobSystem::Wait(const Job* job)
{
	// Wait until the job has completed
	while (HasJobCompleted(job) == false)
	{
		// While waiting, work on any other job
		Job* nextJob = GetJobToExecute();

		if (nextJob != nullptr)
			Execute(nextJob);
	}
}

void JobSystem::EndFrame()
{
	size_t threadCount = workerCount + 1;
	for (size_t i = 0; i < threadCount; ++i)
		jobAllocators[i].Reset();
}

// ===============
// PRIVATE METHODS
// ===============

bool JobSystem::HasJobCompleted(const Job* job)
{
	return job->unfinishedJobs.load() == 0;
}

void JobSystem::Execute(Job* job)
{
	job->function(job, this);
	Finish(job);
}

void JobSystem::Finish(Job* job)
{
	size_t unfinishedJobs = job->unfinishedJobs.fetch_sub(1);

	// fetch_sub returns the previous value, so 1 means the value now is 0
	if (unfinishedJobs == 1 && job->parent != nullptr)
	{
		Finish(job->parent);
	}
}

Job* JobSystem::GetJobToExecute()
{
	JobQueue* queue = GetCurrentThreadJobQueue();

	Job* job = queue->Pop();
	if (job != nullptr)
		return job;

	// No jobs found in current thread queue
	// Try to steal a job from another thread

	const size_t threadCount = workerCount + 1;
	const size_t lastQueueIndex = threadCount - 1;
		
	// Get start index by random number generation
	size_t threadIndex = static_cast<size_t>(Random::Uint(0, lastQueueIndex));
		
	// Try to steal from each thread queue
	for (size_t i = 0; i < threadCount; ++i)
	{
		// Skip current thread queue
		if (threadIndex == currentThreadIndex)
			continue;

		Job* stolenJob = jobQueues[threadIndex].Steal();

		if (stolenJob != nullptr)
			return stolenJob;

		threadIndex = (threadIndex + 1) % threadCount;
	}

	return nullptr;
}

Job* JobSystem::AllocateJob()
{
	return jobAllocators[currentThreadIndex].AllocateJob();
}

JobQueue* JobSystem::GetCurrentThreadJobQueue()
{
	return &jobQueues[currentThreadIndex];
}

struct TestFnData
{
	int64_t result;
	int64_t padding[7];
};

static void EmptyJob(Job* job, JobSystem* jobSystem)
{
}

static int64_t Work()
{
	constexpr int64_t SumCount = 1 << 22;

	int64_t sum = 0;
	for (int64_t i = 0; i < SumCount; ++i)
		sum += i;

	return sum;
}

static void TestJob(Job* job, JobSystem* jobSystem)
{
	KOKKO_PROFILE_SCOPE("TestJob");

	int64_t sum = Work();
	static_cast<TestFnData*>(job->GetPtr())->result = sum;
};

TEST_CASE("JobSystem")
{
	constexpr size_t IterationCount = 5;
	constexpr size_t JobCount = 100;

	int64_t validationResult = Work();

	Allocator* allocator = Allocator::GetDefault();

	void* resultsBuffer = allocator->AllocateAligned(sizeof(TestFnData) * JobCount, KK_CACHE_LINE);
	TestFnData* results = static_cast<TestFnData*>(resultsBuffer);

	for (size_t iter = 0; iter < IterationCount; ++iter)
	{
		std::memset(results, 0, sizeof(TestFnData) * JobCount);

		JobSystem jobSystem(allocator, 3);
		jobSystem.Initialize();

		Job* parent = jobSystem.CreateJob(EmptyJob);

		for (size_t j = 0; j < JobCount; ++j)
		{
			Job* job = jobSystem.CreateJobAsChildWithPtr(TestJob, parent, &results[j]);
			jobSystem.Enqueue(job);
		}

		jobSystem.Enqueue(parent);

		jobSystem.Wait(parent);

		jobSystem.EndFrame();

		jobSystem.Deinitialize();

		for (int j = 0; j < JobCount; ++j)
			CHECK(results[j].result == validationResult);
	}

	allocator->Deallocate(resultsBuffer);
}
