#pragma once

#include "Memory/Allocator.hpp"

namespace kokko
{

class MetricAllocator : public Allocator
{
private:
	Allocator* allocator;
	const char* memoryScopeName;
	std::size_t allocatedSize;
	std::size_t allocatedCount;

public:
	MetricAllocator(const char* memoryScope, Allocator* allocator);
	virtual ~MetricAllocator();

	std::size_t GetTotalAllocationSize() const;
	std::size_t GetTotalAllocationCount() const;
	const char* GetMemoryScopeName() const;

	virtual void* Allocate(size_t size, const char* debugTag = nullptr) override;
	virtual void* AllocateAligned(size_t size, size_t alignment, const char* debugTag = nullptr) override;
	virtual void Deallocate(void* ptr) override;
	virtual size_t GetAllocatedSize(void* ptr) override;
};

} // namespace kokko
