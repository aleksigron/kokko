#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>

class Allocator;
class JobAllocator;
class JobQueue;
class JobSystem;
class JobWorker;

struct Job;
struct JobStorage;

using JobFunction = void(*)(Job*, JobSystem*);

class JobSystem
{
public:
	JobSystem(Allocator* allocator, size_t numWorkers);
	~JobSystem();

	void Initialize();
	void Deinitialize();

	Job* CreateJob(JobFunction function);
	Job* CreateJobWithPtr(JobFunction function, void* ptr);
	Job* CreateJobWithData(JobFunction function, const void* data, size_t size);

	Job* CreateJobAsChild(JobFunction function, Job* parent);
	Job* CreateJobAsChildWithPtr(JobFunction function, Job* parent, void* ptr);
	Job* CreateJobAsChildWithData(JobFunction function, Job* parent, const void* data, size_t size);

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
	
	Job* AllocateJob();

	JobQueue* GetCurrentThreadJobQueue();

private:
	Allocator* allocator;

	JobAllocator* jobAllocators;
	JobQueue* jobQueues;

	size_t workerCount;
	JobWorker* workers;

	std::mutex conditionMutex;
	std::condition_variable jobAddedCondition;

	friend class JobWorker;
};
