#pragma once

#include "StackAllocation.hpp"

#include <cstddef>
#include <cstdint>

/**
* StackAllocator class can be used to create StackAllocation objects that
* automatically release their memory when they're destructed. This can be used
* create dynamically allocated blocks of memory that are practically impossible
* to forget to deallocate.
*/
class StackAllocator
{
private:
	uint8_t* buffer;
	uint8_t* freeStart;
	size_t reservedSize;
	size_t allocatedSize;
	size_t allocBlockSize;

	void Deallocate(const StackAllocation& allocation);

	friend class StackAllocation;

public:
	StackAllocator(size_t reserveSize, size_t allocBlockSize);
	~StackAllocator();

	StackAllocation Allocate(size_t size);
};
