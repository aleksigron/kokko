#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Core/Array.hpp"
#include "Core/Queue.hpp"

using JobFunction = void(*)(void*);

class JobWorker;

struct Job;

class JobSystem
{
public:
	JobSystem(Allocator* allocator, int numWorkers);
	~JobSystem();

	void Initialize();
	void Deinitialize();

	Job* CreateJob(JobFunction function, void* userData);

	void Enqueue(size_t count, Job** jobs);

	void Wait(const Job* job);

	// NOTE: Jobs can NOT be running when calling this
	void ReleaseCompletedJobs();

private:
	static bool HasJobCompleted(const Job* job);
	
	// Lock on queueMutex must be acquired to call this
	Job* GetJobToExecute();

	void Execute(Job* job);
	void Finish(Job* job);

	Job* AllocateJob();
	void ReleaseJob(Job* job);

	Allocator* allocator;

	int workerCount;

	JobWorker* workers;
	Queue<Job*> queue;

	static const size_t MaxJobsToDelete = 1 << 10;
	Job** jobsToDelete;
	std::atomic_size_t jobsToDeleteCount;

	std::mutex queueMutex;
	std::condition_variable jobAddedCondition;


	friend class JobWorker;
};
