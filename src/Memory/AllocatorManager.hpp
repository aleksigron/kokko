#pragma once

#include <cstdlib>

#include "Memory/Allocator.hpp"

class MetricAllocator;

class AllocatorManager
{
private:
	Allocator* alloc;

	MetricAllocator** scopes;
	unsigned int scopeCount;
	unsigned int scopeAllocated;

public:
	AllocatorManager(Allocator* allocator);
	~AllocatorManager();

	/// <summary>
	/// Create a named allocator scope that will enable gathering memory
	/// statistics.
	/// </summary>
	/// <param name="name">Name for the memory scope</param>
	/// <param name="baseAllocator">Allocator to use as base allocator</param>
	/// <returns>
	/// Allocator that will gather memory statistics. Call <see cref="DestroyAllocatorScope(Allocator*)"/>
	/// with this return value when your done with the allocator.
	/// </returns>
	Allocator* CreateAllocatorScope(const char* name, Allocator* baseAllocator, bool tracing = false);

	/// <summary>
	/// Destroy a previously created allocator scope.
	/// </summary>
	/// <param name="allocator">Allocator to destroy</param>
	void DestroyAllocatorScope(Allocator* allocator);

	unsigned int GetMemoryTrackingScopeCount();

	const char* GetNameForScopeIndex(unsigned int index) const;
	std::size_t GetAllocatedSizeForScopeIndex(unsigned int index) const;
	std::size_t GetAllocationCountForScopeIndex(unsigned int index) const;
};
