#include "Memory/MetricAllocator.hpp"

#include <cassert>

MetricAllocator::MetricAllocator(const char* memoryScope, Allocator* allocator):
	allocator(allocator),
	memoryScopeName(memoryScope),
	allocatedSize(0),
	allocatedCount(0)
{
}

MetricAllocator::~MetricAllocator()
{
	assert(allocatedSize == 0);
	assert(allocatedCount == 0);
}

std::size_t MetricAllocator::GetTotalAllocationSize() const
{
	return allocatedSize;
}

std::size_t MetricAllocator::GetTotalAllocationCount() const
{
	return allocatedCount;
}

const char* MetricAllocator::GetMemoryScopeName() const
{
	return memoryScopeName;
}

void* MetricAllocator::Allocate(size_t size, const char* debugTag)
{
	void* result = allocator->Allocate(size, debugTag);

	if (result != nullptr)
	{
		allocatedSize += size;
		allocatedCount += 1;
	}

	return result;
}

void* MetricAllocator::AllocateAligned(size_t size, size_t alignment, const char* debugTag)
{
	void* result = allocator->AllocateAligned(size, alignment);

	if (result != nullptr)
	{
		allocatedSize += size;
		allocatedCount += 1;
	}

	return result;
}

void MetricAllocator::Deallocate(void* ptr)
{
	if (ptr != nullptr)
	{
		size_t size = allocator->GetAllocatedSize(ptr);

		allocatedSize -= size;
		allocatedCount -= 1;

		allocator->Deallocate(ptr);
	}
}

std::size_t MetricAllocator::GetAllocatedSize(void* ptr)
{
	return allocator->GetAllocatedSize(ptr);
}
