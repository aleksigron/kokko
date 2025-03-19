#include "Engine/JobWorker.hpp"

#include <cassert>

#include "Engine/Job.hpp"
#include "Engine/JobSystem.hpp"

namespace kokko
{

JobWorker::JobWorker(size_t threadIndex, JobSystem* jobSystem) :
	threadIndex(threadIndex),
	jobSystem(jobSystem),
	exitRequested(false)
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
	JobSystem::currentThreadIndex = threadIndex;

	bool exit = false;

	while (exit == false)
	{
		static const size_t MaxGetJobTryCount = 10;
		size_t getJobTryCount = 0;

		Job* job;

		do
		{
			job = jobSystem->GetJobToExecute();

			if (job != nullptr)
			{
				getJobTryCount = 0;
				jobSystem->Execute(job);
			}
			else
			{
				getJobTryCount += 1;
			}
		} while (getJobTryCount < MaxGetJobTryCount);
		// GetJobToExecute can return nullptr even when there are jobs remaining.
		// We only start waiting on the condition variable if we failed to get a job
		// MaxGetJobTryCount times in a row.

		exit = exitRequested.load();
		if (exit == true)
			break;

		// Allow wake-ups even when we don't know if there is work to do
		// That means we try get a job but we will return here if no jobs are available
		std::unique_lock<std::mutex> lock(jobSystem->conditionMutex);
		{
			KOKKO_PROFILE_SCOPE("CondWait");
			jobSystem->jobAddedCondition.wait(lock);
		}

		exit = exitRequested.load();
	}

	// We need to make sure no one is accidentally modifying the thread index
	assert(JobSystem::currentThreadIndex == threadIndex);
}

} // namespace kokko
