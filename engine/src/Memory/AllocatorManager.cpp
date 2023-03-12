#include "Memory/AllocatorManager.hpp"

#include <cstring>

#include "Memory/MetricAllocator.hpp"
#include "Memory/TraceAllocator.hpp"

AllocatorManager::AllocatorManager(Allocator* allocator) :
	alloc(allocator),
	scopes(nullptr),
	scopeCount(0),
	scopeAllocated(0)
{
}

AllocatorManager::~AllocatorManager()
{
	// Delete any not deallocated allocators
	for (unsigned int i = 0; i < scopeCount; ++i)
	{
		this->alloc->MakeDelete(scopes[i]);
	}
}

Allocator* AllocatorManager::CreateAllocatorScope(const char* name, Allocator* baseAllocator, bool tracing)
{
	if (scopeCount == scopeAllocated)
	{
		// Reallocate

		unsigned int newAllocated = scopeAllocated > 0 ? scopeAllocated * 2 : 32;
		void* newBuffer = this->alloc->Allocate(sizeof(MetricAllocator*) * newAllocated, "AllocatorManager.scopes");

		if (scopeCount > 0)
		{
			std::memcpy(newBuffer, scopes, sizeof(MetricAllocator*) * scopeCount);
			this->alloc->Deallocate(scopes);
		}

		scopes = static_cast<MetricAllocator**>(newBuffer);
		scopeAllocated = newAllocated;
	}

	MetricAllocator* proxyAllocator = nullptr;
	if (tracing)
		proxyAllocator = this->alloc->MakeNew<TraceAllocator>(name, baseAllocator);
	else
		proxyAllocator = this->alloc->MakeNew<MetricAllocator>(name, baseAllocator);

	scopes[scopeCount] = proxyAllocator;
	scopeCount += 1;

	return proxyAllocator;
}

void AllocatorManager::DestroyAllocatorScope(Allocator* allocator)
{
	// Find right allocator
	for (unsigned int i = 0; i < scopeCount; ++i)
	{
		if (scopes[i] == allocator)
		{
			this->alloc->MakeDelete(scopes[i]);

			if (i != scopeCount - 1)
			{
				// Swap last scope to deleted ones place
				scopes[i] = scopes[scopeCount - 1];
			}

			scopeCount -= 1;

			break;
		}
	}
}

unsigned int AllocatorManager::GetMemoryTrackingScopeCount()
{
	return scopeCount;
}

const char* AllocatorManager::GetNameForScopeIndex(unsigned int index) const
{
	return scopes[index]->GetMemoryScopeName();
}

std::size_t AllocatorManager::GetAllocatedSizeForScopeIndex(unsigned int index) const
{
	return scopes[index]->GetTotalAllocationSize();
}

std::size_t AllocatorManager::GetAllocationCountForScopeIndex(unsigned int index) const
{
	return scopes[index]->GetTotalAllocationCount();
}
