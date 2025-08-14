#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "Engine/Job.hpp"
#include "Engine/JobFunction.hpp"

namespace kokko
{

class Allocator;
class JobAllocator;
class JobQueue;
class JobWorker;

class JobSystem
{
public:
	JobSystem(Allocator* allocator, size_t numWorkers);
	~JobSystem();

	void Initialize();
	void Deinitialize();

	Job* CreateJob(JobFunction function);
	Job* CreateJobWithPtr(JobFunction function, void* ptr);
	Job* CreateJobWithBuffer(JobFunction function, const void* data, size_t size);

	Job* CreateJobAsChild(Job* parent, JobFunction function);
	Job* CreateJobAsChildWithPtr(Job* parent, JobFunction function, void* ptr);
	Job* CreateJobAsChildWithBuffer(Job* parent, JobFunction function, const void* data, size_t size);

	template <typename T>
	Job* CreateJobWithData(JobFunction function, const T& data)
	{
		static_assert(sizeof(T) <= sizeof(Job::data));
		return CreateJobWithBuffer(function, &data, sizeof(data));
	}

	template <typename T>
	Job* CreateJobAsChildWithData(Job* parent, JobFunction function, const T& data)
	{
		static_assert(sizeof(T) <= sizeof(Job::data));
		return CreateJobAsChildWithBuffer(parent, function, &data, sizeof(data));
	}

	void Enqueue(Job* job);

	void Wait(const Job* job);

	void EndFrame();

	static const size_t MaxJobsPerThreadPerFrame = 1 << 12;
	static const size_t ThreadIndexMainThread = 0;

	static thread_local size_t currentThreadIndex;

private:
	static bool HasJobCompleted(const Job* job);

	void Execute(Job* job);
	void Finish(Job* job);

	Job* GetJobToExecute();

	bool IsWorkAvailable();

	Job* AllocateJob();

	JobQueue* GetCurrentThreadJobQueue();

private:
	Allocator* allocator;

	JobAllocator* jobAllocators;
	JobQueue* jobQueues;

	size_t workerCount;
	JobWorker* workers;

	std::mutex conditionMutex;
	std::condition_variable workerNotifyCondition;

	friend class JobWorker;
};

} // namespace kokko
