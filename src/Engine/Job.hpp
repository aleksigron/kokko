#pragma once

#include <atomic>

#include "Engine/JobSystem.hpp"

struct Job
{
	JobFunction function;
	Job* parent;
	std::atomic_size_t unfinishedJobs;
	uint8_t padding[64 - (sizeof(JobFunction) + sizeof(Job*) + sizeof(std::atomic_size_t))];

	void* GetPtr()
	{
		return *reinterpret_cast<void**>(padding);
	}

	template <typename T>
	void GetDataAs(T& valueOut)
	{
		static_assert(sizeof(T) <= sizeof(padding));
		std::memcpy(&value, padding, sizeof(T));
	}
};
