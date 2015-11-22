#pragma once

#include "StackAllocation.hpp"

#include <cstddef>
#include <cstdint>

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
