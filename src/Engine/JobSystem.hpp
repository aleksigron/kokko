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

	void AddJob(JobFunction function, void* userData);

private:
	Allocator* allocator;

	Queue<Job*> queue;
	JobWorker* workers;

	std::mutex queueMutex;
	std::condition_variable jobAddedCondition;

	int workerCount;

	friend class JobWorker;
};
