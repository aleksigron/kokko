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
	jobBuffer(nullptr),
	jobBufferHead(0),
	jobsDuringFrame(0)
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

	if (jobBuffer == nullptr)
	{
		void* buf = allocator->AllocateAligned(sizeof(Job) * MaxJobsPerFrame, KK_CACHE_LINE);
		jobBuffer = static_cast<Job*>(buf);
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

	if (jobBuffer != nullptr)
	{
		allocator->Deallocate(jobBuffer);
		jobBuffer = nullptr;
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

void JobSystem::EndFrame()
{
	assert(jobsDuringFrame < MaxJobsPerFrame);
	jobsDuringFrame.store(0);
}

bool JobSystem::HasJobCompleted(const Job* job)
{
	return job->unfinishedJobs.load() == 0;
}

Job* JobSystem::GetJobToExecute()
{
	Job* job = nullptr;
	queue.TryPop(job);
	return job;
}

void JobSystem::Execute(Job* job)
{
	job->function(job);
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

Job* JobSystem::AllocateJob()
{
	size_t index = jobBufferHead.fetch_add(1);

	jobsDuringFrame.fetch_add(1);

	return &jobBuffer[index];
}

struct TestFnData
{
	int64_t result;
	int64_t padding[7];
};

static void EmptyJob(Job* job)
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

static void TestJob(Job* job)
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
			jobSystem.Enqueue(1, &job);
		}

		jobSystem.Enqueue(1, &parent);

		jobSystem.Wait(parent);

		jobSystem.EndFrame();

		jobSystem.Deinitialize();

		for (int j = 0; j < JobCount; ++j)
			CHECK(results[j].result == validationResult);
	}

	allocator->Deallocate(resultsBuffer);
}
