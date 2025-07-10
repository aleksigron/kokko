#include "Engine/JobSystem.hpp"

#include <cassert>
#include <cstddef>
#include <new>

#include "doctest/doctest.h"

#include "Core/Core.hpp"

#include "Engine/JobAllocator.hpp"
#include "Engine/JobHelpers.hpp"
#include "Engine/JobQueue.hpp"
#include "Engine/JobWorker.hpp"

#include "Math/Math.hpp"
#include "Math/Random.hpp"

#include "Memory/Allocator.hpp"

namespace kokko
{

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
	KOKKO_PROFILE_FUNCTION();

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
	KOKKO_PROFILE_FUNCTION();

	size_t threadCount = workerCount + 1;

	if (workers != nullptr)
	{
		for (int i = 0; i < workerCount; ++i)
			workers[i].RequestExit();

		{
			// Lock must be acquired before notifying the condition 
			std::lock_guard<std::mutex> lock(conditionMutex);
			jobAddedCondition.notify_all();
		}

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
	return CreateJobWithBuffer(function, nullptr, 0);
}

Job* JobSystem::CreateJobWithPtr(JobFunction function, void* ptr)
{
	return CreateJobWithBuffer(function, &ptr, sizeof(void*));
}

Job* JobSystem::CreateJobWithBuffer(JobFunction function, const void* data, size_t size)
{
	assert(size <= sizeof(Job::data));

	Job* job = AllocateJob();
	job->function = function;
	job->parent = nullptr;
	job->unfinishedJobs = 1;

	if (size > 0)
	{
		std::memcpy(job->data, data, size);
	}

	return job;
}

Job* JobSystem::CreateJobAsChild(Job* parent, JobFunction function)
{
	return CreateJobAsChildWithBuffer(parent, function, nullptr, 0);
}

Job* JobSystem::CreateJobAsChildWithPtr(Job* parent, JobFunction function, void* ptr)
{
	return CreateJobAsChildWithBuffer(parent, function, &ptr, sizeof(void*));
}

Job* JobSystem::CreateJobAsChildWithBuffer(Job* parent, JobFunction function, const void* data, size_t size)
{
	parent->unfinishedJobs.fetch_add(1);

	Job* job = AllocateJob();
	job->function = function;
	job->parent = parent;
	job->unfinishedJobs = 1;

	if (size > 0)
	{
		std::memcpy(job->data, data, size);
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
	size_t threadIndex = static_cast<size_t>(Random::Uint64(0, lastQueueIndex));

	// Try to steal from each thread queue
	for (size_t i = 0; i < threadCount; ++i)
	{
		// Skip current thread queue
		if (threadIndex != currentThreadIndex)
		{
			// Give hint to allow other threads to work
			std::this_thread::yield();

			Job* stolenJob = jobQueues[threadIndex].Steal();

			if (stolenJob != nullptr)
				return stolenJob;
		}

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

struct TestJobConstantData
{
	float deltaTime;
	float dampenPerSecond;
	int iterations;
};

struct TestJobData
{
	float pos[3];
	float vel[3];
};

static void TestJob(TestJobConstantData* constant, TestJobData* data, size_t count)
{
	KOKKO_PROFILE_SCOPE("TestJob");

	const float dt = constant->deltaTime;
	const float hdt = dt * 0.5f;
	const float dampen = Math::DampenMultiplier(constant->dampenPerSecond, dt);

	const int iterations = constant->iterations;

	for (size_t iter = 0; iter < iterations; ++iter)
	{
		for (size_t index = 0; index < count; ++index)
		{
			float velx = data[index].vel[0];
			float vely = data[index].vel[1];
			float velz = data[index].vel[2];

			float posx = data[index].pos[0] + velx * hdt;
			float posy = data[index].pos[1] + vely * hdt;
			float posz = data[index].pos[2] + velz * hdt;

			velx = velx * dampen;
			vely = vely * dampen;
			velz = velz * dampen;

			data[index].pos[0] = posx + velx * hdt;
			data[index].pos[1] = posy + vely * hdt;
			data[index].pos[2] = posz + velz * hdt;
		}
	}
};

static void ResetTestData(TestJobData* data, size_t count)
{
	KOKKO_PROFILE_SCOPE("ResetTestData");

	for (size_t i = 0; i < count; ++i)
	{
		data[i].pos[0] = 0.0f;
		data[i].pos[1] = 0.0f;
		data[i].pos[2] = 0.0f;

		data[i].vel[0] = 4.0f;
		data[i].vel[1] = 4.0f;
		data[i].vel[2] = 4.0f;
	}
}

TEST_CASE("JobSystem")
{
	constexpr size_t IterationCount = 5;
	constexpr size_t DataCount = 5'000'000;

	TestJobConstantData jobConstantData;
	jobConstantData.deltaTime = 0.01f;
	jobConstantData.dampenPerSecond = 0.8f;
	jobConstantData.iterations = 20;

	TestJobData validationResult;
	ResetTestData(&validationResult, 1);
	TestJob(&jobConstantData, &validationResult, 1);

	Allocator* allocator = Allocator::GetDefault();

	void* resultsBuffer = allocator->Allocate(sizeof(TestJobData) * DataCount);
	TestJobData* results = static_cast<TestJobData*>(resultsBuffer);

	for (size_t iter = 0; iter < IterationCount; ++iter)
	{
		ResetTestData(results, DataCount);

		JobSystem jobSystem(allocator, 7);
		Job* job;

		{
			KOKKO_PROFILE_SCOPE("JobSystem init and enqueue work");

			jobSystem.Initialize();

			size_t splitCount = 1 << 12;
			job = JobHelpers::CreateParallelFor(&jobSystem, &jobConstantData, results, DataCount, TestJob, splitCount);

			jobSystem.Enqueue(job);
		}

		jobSystem.Wait(job);

		{
			KOKKO_PROFILE_SCOPE("JobSystem deinit and check results");

			jobSystem.EndFrame();

			jobSystem.Deinitialize();

			bool resultsMatch = true;
			for (int j = 0; j < DataCount; ++j)
			{
				if (results[j].pos[0] != validationResult.pos[0] ||
					results[j].pos[1] != validationResult.pos[1] ||
					results[j].pos[2] != validationResult.pos[2] ||
					results[j].vel[0] != validationResult.vel[0] ||
					results[j].vel[1] != validationResult.vel[1] ||
					results[j].vel[2] != validationResult.vel[2])
				{
					resultsMatch = false;
					break;
				}
			}

			CHECK(resultsMatch == true);
		}
	}

	allocator->Deallocate(resultsBuffer);
}

} // namespace kokko
