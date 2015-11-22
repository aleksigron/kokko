#include "StackAllocator.hpp"

#include <cassert>

#include "VirtualMemory.hpp"

StackAllocator::StackAllocator(size_t reserveSize, size_t allocBlockSize):
allocatedSize(0),
allocBlockSize(allocBlockSize)
{
	void* mem = VirtualMemory::ReserveAddressSpace(reserveSize);

	this->buffer = static_cast<uint8_t*>(mem);
	this->freeStart = this->buffer;

	if (mem != nullptr)
		this->reservedSize = reserveSize;
}

StackAllocator::~StackAllocator()
{
	// Make sure all allocations are freed before StackAllocator is destroyed
	assert(this->freeStart == this->buffer);

	if (this->buffer != nullptr)
		VirtualMemory::FreeAddressSpace(this->buffer, this->reservedSize);
}

StackAllocation StackAllocator::Allocate(size_t size)
{
	StackAllocation allocation;

	size_t requiredSize = (freeStart - buffer) + size;

	// There is enough reserved space so this allocation can be satisfied
	if (requiredSize <= this->reservedSize)
	{
		// There isn't enough allocated memory
		if (requiredSize > this->allocatedSize)
		{
			uint8_t* mem = buffer + allocatedSize;

			VirtualMemory::CommitReservedSpace(mem, allocBlockSize);
		}

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

	size_t newUsedSize = (freeStart - buffer) - allocation.size;
	size_t newAllocatedSize = allocatedSize - allocBlockSize;

	if (newUsedSize < newAllocatedSize)
	{
		VirtualMemory::DecommitReservedSpace(buffer + newAllocatedSize, allocBlockSize);
	}

	this->freeStart = allocation.data;
}
