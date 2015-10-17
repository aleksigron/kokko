#pragma once

#include "StackAllocation.hpp"

#include <cstddef>
#include <cstdint>

class StackAllocator
{
private:
	uint8_t* buffer;
	uint8_t* freeStart;
	size_t allocatedSize;

	void Deallocate(const StackAllocation& allocation);

	friend class StackAllocation;

public:
	StackAllocator(size_t allocatedSize);
	~StackAllocator();

	StackAllocation Allocate(size_t size);
};
