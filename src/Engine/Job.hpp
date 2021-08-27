#pragma once

#include <atomic>

#include "Engine/JobSystem.hpp"

struct Job
{
	JobFunction function;
	void* userData;
	std::atomic_int unfinishedJobs;
	uint8_t padding[64 - (sizeof(JobFunction) + sizeof(void*) + sizeof(std::atomic_int))];
};
