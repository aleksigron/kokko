#pragma once

#include <atomic>

#include "Core/Core.hpp"

#include "Engine/JobFunction.hpp"

namespace kokko
{

struct Job
{
	JobFunction function;
	Job* parent;
	std::atomic_size_t unfinishedJobs;

	static const size_t CL = KK_CACHE_LINE;
	static const size_t MemberBytes = sizeof(JobFunction) + sizeof(Job*) + sizeof(std::atomic_size_t);
	uint8_t data[(MemberBytes + CL - 1) / CL * CL - MemberBytes];

	void* GetDataAsPtr()
	{
		return *reinterpret_cast<void**>(data);
	}

	template <typename T>
	T* GetPtrToData()
	{
		static_assert(sizeof(T) <= sizeof(data));
		return reinterpret_cast<T*>(&data);
	}

	template <typename T>
	void GetDataAs(T& valueOut)
	{
		static_assert(sizeof(T) <= sizeof(data));
		std::memcpy(&valueOut, data, sizeof(T));
	}
};

} // namespace kokko
