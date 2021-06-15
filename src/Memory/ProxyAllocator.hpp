#pragma once

#include "Memory/Allocator.hpp"

class ProxyAllocator : public Allocator
{
private:
	Allocator* allocator;
	const char* memoryScopeName;
	std::size_t allocatedSize;
	std::size_t allocatedCount;

public:
	ProxyAllocator(const char* memoryScope, Allocator* allocator);
	virtual ~ProxyAllocator();

	std::size_t GetTotalAllocationSize() const;
	std::size_t GetTotalAllocationCount() const;
	const char* GetMemoryScopeName() const;

	virtual void* Allocate(std::size_t size, const char* debugTag = nullptr) override;
	virtual void Deallocate(void* ptr) override;
	virtual std::size_t GetAllocatedSize(void* ptr) override;
};
