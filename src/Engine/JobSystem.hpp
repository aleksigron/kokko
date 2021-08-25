#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>

#include "Core/Array.hpp"
#include "Core/Queue.hpp"

#include "Engine/JobWorker.hpp"

using JobFunction = void(*)(void*);

struct Job
{
	JobFunction function;
	void* userData;
	uint8_t padding[64 - sizeof(void*) * 2];
};

class JobSystem
{
public:
	JobSystem(Allocator* allocator, int numWorkers);
	~JobSystem();

	void Initialize();
	void Deinitialize();

	void AddJob(const Job& job);

private:
	Queue<Job> queue;
	Array<JobWorker> workers;

	std::mutex queueMutex;
	std::condition_variable jobAddedCondition;

	int workerCount;

	friend class JobWorker;
};
