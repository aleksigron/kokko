#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Core/Array.hpp"
#include "Core/Queue.hpp"

class JobWorker;

struct Job;

using JobFunction = void(*)(Job*);

class JobSystem
{
public:
	JobSystem(Allocator* allocator, int numWorkers);
	~JobSystem();

	void Initialize();
	void Deinitialize();

	Job* CreateJob(JobFunction function);
	Job* CreateJobWithPtr(JobFunction function, void* ptr);
	Job* CreateJobWithData(JobFunction function, const void* data, size_t size);

	Job* CreateJobAsChild(JobFunction function, Job* parent);
	Job* CreateJobAsChildWithPtr(JobFunction function, Job* parent, void* ptr);
	Job* CreateJobAsChildWithData(JobFunction function, Job* parent, const void* data, size_t size);

	void Enqueue(size_t count, Job** jobs);

	void Wait(const Job* job);

	void EndFrame();

private:
	static bool HasJobCompleted(const Job* job);
	
	// Lock on queueMutex must be acquired to call this
	Job* GetJobToExecute();

	void Execute(Job* job);
	void Finish(Job* job);

	Job* AllocateJob();

private:
	Allocator* allocator;

	int workerCount;
	JobWorker* workers;

	Queue<Job*> queue;

	static const size_t MaxJobsPerFrame = 1 << 12;
	Job* jobBuffer;
	std::atomic_size_t jobBufferHead;
	std::atomic_size_t jobsDuringFrame;

	std::mutex queueMutex;
	std::condition_variable jobAddedCondition;

	friend class JobWorker;
};
