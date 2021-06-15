#include "Memory/TraceAllocator.hpp"

#include <cassert>

TraceAllocator::TraceAllocator(const char* memoryScope, Allocator* allocator) :
	MetricAllocator(memoryScope, allocator),
	allocations(allocator)
{
}

TraceAllocator::~TraceAllocator()
{
	if (allocations.GetCount() > 0)
		OutputAllocations(stdout);

	assert(allocations.GetCount() == 0);
}

void* TraceAllocator::Allocate(std::size_t size, const char* debugTag)
{
	void* ptr = MetricAllocator::Allocate(size);
	allocations.Insert(AllocationInfo{ ptr, debugTag });
	return ptr;
}

void TraceAllocator::Deallocate(void* ptr)
{
	if (ptr != nullptr)
	{
		int index = allocations.Find(AllocationInfo{ ptr, nullptr });
		assert(index >= 0);
		allocations.Remove(index);
	}

	MetricAllocator::Deallocate(ptr);
}

void TraceAllocator::OutputAllocations(FILE* stream)
{
	fprintf(stream, "TraceAllocator (%s):\n", GetMemoryScopeName());

	if (allocations.GetCount() > 0)
	{
		fprintf(stream, "\t%zu allocations, %zu bytes total\n", GetTotalAllocationCount(), GetTotalAllocationSize());

		for (const AllocationInfo& alloc : allocations)
		{
			const char* tag = alloc.tag != nullptr ? alloc.tag : "Unnamed";
			fprintf(stream, "\t%p: %8zu B, %s\n", alloc.ptr, GetAllocatedSize(alloc.ptr), tag);
		}
	}
	else
		fprintf(stream, "\tNo allocations");
}
