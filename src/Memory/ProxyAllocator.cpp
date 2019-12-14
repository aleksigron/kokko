#include "Memory/ProxyAllocator.hpp"

#include <cassert>

ProxyAllocator::ProxyAllocator(const char* memoryScope, Allocator* allocator):
	allocator(allocator),
	memoryScopeName(memoryScope),
	allocatedSize(0),
	allocatedCount(0)
{
}

ProxyAllocator::~ProxyAllocator()
{
	assert(allocatedSize == 0);
	assert(allocatedCount == 0);
}

std::size_t ProxyAllocator::GetTotalAllocationSize() const
{
	return allocatedSize;
}

std::size_t ProxyAllocator::GetTotalAllocationCount() const
{
	return allocatedCount;
}

const char* ProxyAllocator::GetMemoryScopeName() const
{
	return memoryScopeName;
}

void* ProxyAllocator::Allocate(std::size_t size)
{
	void* result = allocator->Allocate(size);

	if (result != nullptr)
	{
		allocatedSize += size;
		allocatedCount += 1;
	}

	return result;
}

void ProxyAllocator::Deallocate(void* ptr)
{
	if (ptr != nullptr)
	{
		std::size_t size = allocator->GetAllocatedSize(ptr);

		allocatedSize -= size;
		allocatedCount -= 1;

		allocator->Deallocate(ptr);
	}
}

std::size_t ProxyAllocator::GetAllocatedSize(void* ptr)
{
	return allocator->GetAllocatedSize(ptr);
}
