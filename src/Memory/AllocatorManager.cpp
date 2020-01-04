#include "Memory/AllocatorManager.hpp"

#include <cstring>

#include "Memory/ProxyAllocator.hpp"

AllocatorManager::AllocatorManager(Allocator* allocator) :
	alloc(allocator),
	scopes(nullptr),
	scopeCount(0),
	scopeAllocated(0)
{
}

AllocatorManager::~AllocatorManager()
{
	
}

Allocator* AllocatorManager::CreateAllocatorScope(const char* name, Allocator* baseAllocator)
{
	if (scopeCount == scopeAllocated)
	{
		// Reallocate

		unsigned int newAllocated = scopeAllocated > 0 ? scopeAllocated * 2 : 32;
		void* newBuffer = this->alloc->Allocate(sizeof(ProxyAllocator*) * newAllocated);

		if (scopeCount > 0)
		{
			std::memcpy(newBuffer, scopes, sizeof(ProxyAllocator*) * scopeCount);
			this->alloc->Deallocate(scopes);
		}

		scopes = static_cast<ProxyAllocator**>(newBuffer);
		scopeAllocated = newAllocated;
	}

	ProxyAllocator* proxy = this->alloc->MakeNew<ProxyAllocator>(name, baseAllocator);

	scopes[scopeCount] = proxy;
	scopeCount += 1;

	return proxy;
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
