#include "StackAllocator.h"

#include <cassert>

StackAllocator::StackAllocator(size_t allocatedSize)
{
	this->buffer = new uint8_t[allocatedSize];
	this->freeStart = this->buffer;
	this->allocatedSize = allocatedSize;
}

StackAllocator::~StackAllocator()
{
	// Make sure all allocations are freed before StackAllocator is destroyed
	assert(this->freeStart == this->buffer);

	delete this->buffer;
}

StackAllocation StackAllocator::Allocate(size_t size)
{
	StackAllocation allocation;

	// Allocation can be satisfied
	if (freeStart - buffer + size <= this->allocatedSize)
	{
		allocation.allocator = this;
		allocation.data = freeStart;
		allocation.size = size;

		freeStart += size;
	}
	else
	{
		allocation.allocator = nullptr;
		allocation.data = nullptr;
		allocation.size = 0;
	}

	return allocation;
}

void StackAllocator::Deallocate(const StackAllocation& allocation)
{
	// Make sure this is the top-most allocation
	assert(allocation.data + allocation.size == this->freeStart);

	this->freeStart = allocation.data;
}
