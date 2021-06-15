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

void* MetricAllocator::Allocate(std::size_t size, const char* debugTag)
{
	void* result = allocator->Allocate(size);

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
		std::size_t size = allocator->GetAllocatedSize(ptr);

		allocatedSize -= size;
		allocatedCount -= 1;

		allocator->Deallocate(ptr);
	}
}

std::size_t MetricAllocator::GetAllocatedSize(void* ptr)
{
	return allocator->GetAllocatedSize(ptr);
}
