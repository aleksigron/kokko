#pragma once

#include "Core/SortedArray.hpp"

#include "Memory/ProxyAllocator.hpp"

class TraceAllocator : public ProxyAllocator
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

	SortedArray<AllocationInfo> allocations;

public:
	TraceAllocator(const char* memoryScope, Allocator* allocator);
	virtual ~TraceAllocator();

	virtual void* Allocate(std::size_t size, const char* debugTag = nullptr) override;
	virtual void Deallocate(void* ptr) override;

	void OutputAllocations(FILE* stream);
};
