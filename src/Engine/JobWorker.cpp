#include "Engine/JobWorker.hpp"

#include <cstring>

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

JobWorker::JobWorker(JobSystem* jobSystem) : jobSystem(jobSystem), exitRequested(false)
{
	std::memset(padding, 0, sizeof(padding));
}

void JobWorker::StartThread()
{
	if (thread.joinable() == false)
		thread = std::thread(&JobWorker::ThreadMain, this);
}

void JobWorker::RequestExit()
{
	exitRequested.store(true);
}

void JobWorker::WaitToExit()
{
	if (thread.joinable())
		thread.join();
}

void JobWorker::ThreadMain()
{
	std::unique_lock<std::mutex> lock(jobSystem->queueMutex);

	bool exit = false;

	while (exit == false)
	{
		for (;;)
		{
			Job* job = jobSystem->GetJobToExecute();
			if (job != nullptr)
			{
				lock.unlock();

				jobSystem->Execute(job);

				lock.lock();
			}
			else
			{
				break;
			}
		}

		// Make sure there are jobs in the queue before we start working
		while (jobSystem->queue.GetCount() == 0 && exit == false)
		{
			jobSystem->jobAddedCondition.wait(lock);
			exit = exitRequested.load();
		}
	}
}
