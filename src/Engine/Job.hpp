#pragma once

#include <atomic>

#include "Core/Core.hpp"

#include "Engine/JobSystem.hpp"

struct Job
{
	JobFunction function;
	Job* parent;
	std::atomic_size_t unfinishedJobs;

	static const size_t CL = KK_CACHE_LINE;
	static const size_t MemberBytes = sizeof(JobFunction) + sizeof(Job*) + sizeof(std::atomic_size_t);
	uint8_t padding[(MemberBytes + CL - 1) / CL * CL - MemberBytes];

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
