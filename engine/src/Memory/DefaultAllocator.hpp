#pragma once

#include "Memory/Allocator.hpp"

class DefaultAllocator : public Allocator
{
private:
	static const size_t DefaultMinAlign = 16;
	static const size_t MetadataSize = sizeof(size_t) * 2;

public:
	virtual void* Allocate(size_t size, const char* debugTag = nullptr) override;
	virtual void* AllocateAligned(size_t size, size_t alignment, const char* debugTag = nullptr) override;
	virtual void Deallocate(void* ptr) override;
	virtual size_t GetAllocatedSize(void* ptr) override;
};
