#pragma once

#include <cstddef>
#include <cstdint>

class StackAllocator;

class StackAllocation
{
private:
	StackAllocator* allocator;

	friend class StackAllocator;

public:
	uint8_t* data;
	size_t size;

	StackAllocation();
	StackAllocation(const StackAllocation& other) = delete;
	StackAllocation(StackAllocation&& other);
	~StackAllocation();
};