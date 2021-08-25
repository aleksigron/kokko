#pragma once

#include <thread>

class JobSystem;

class JobWorker
{
public:
	JobWorker();
	JobWorker(JobSystem* jobSystem);

	void StartThread();

	void RequestExit();
	void WaitToExit();

private:
	std::thread thread;
	JobSystem* jobSystem;
	bool exitRequested;

	static void ThreadFunc(JobWorker* self);
};

