#pragma once

#include <cstdio>

#include "Core/SortedArray.hpp"

#include "Memory/MetricAllocator.hpp"

namespace kokko
{

class TraceAllocator : public MetricAllocator
{
private:
	struct AllocationInfo
	{
		void* ptr;
		const char* tag;

		bool operator<(const AllocationInfo& other) const { return ptr < other.ptr; }
		bool operator>(const AllocationInfo& other) const { return ptr > other.ptr; }
		bool operator==(const AllocationInfo& other) const { return ptr == other.ptr; }
	};

	kokko::SortedArray<AllocationInfo> allocations;

public:
	TraceAllocator(const char* memoryScope, Allocator* allocator);
	virtual ~TraceAllocator();

	virtual void* Allocate(size_t size, const char* debugTag = nullptr) override;
	virtual void* AllocateAligned(size_t size, size_t alignment, const char* debugTag = nullptr) override;
	virtual void Deallocate(void* ptr) override;

	void OutputAllocations(FILE* stream);
};

} // namespace kokko
