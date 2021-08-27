#include "Engine/JobWorker.hpp"

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

JobWorker::JobWorker() : jobSystem(nullptr), exitRequested(false)
{
}

JobWorker::JobWorker(JobSystem* jobSystem) : jobSystem(jobSystem), exitRequested(false)
{
}

void JobWorker::StartThread()
{
	if (thread.joinable() == false)
		thread = std::thread(ThreadFunc, this);
}

void JobWorker::RequestExit()
{
	exitRequested = true;
}

void JobWorker::WaitToExit()
{
	if (thread.joinable())
		thread.join();
}

void JobWorker::ThreadFunc(JobWorker* self)
{
	Queue<Job*>& jobQueue = self->jobSystem->queue;

	for (;;)
	{
		std::unique_lock<std::mutex> lock{ self->jobSystem->queueMutex };

		// Make sure there are jobs in the queue before we start working
		while (jobQueue.GetCount() == 0)
		{
			self->jobSystem->jobAddedCondition.wait(lock);
		}

		for (;;)
		{
			Job* job;
			
			if (jobQueue.TryPop(job))
			{
				lock.unlock();

				job->function(job->userData);

				lock.lock();
			}

			// TODO: Check if worker should exit

			// Check if there's still work to do
			// If not, go back to waiting on the condition variable
			// std::unique_lock automatically releases mutex when destructed
			if (jobQueue.GetCount() == 0 /* || self->exitRequested */)
				break;
		}

		if (self->exitRequested)
			break;

	}
}
