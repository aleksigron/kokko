#pragma once

#include "Memory/Allocator.hpp"

class DefaultAllocator : public Allocator
{
private:
	static const std::size_t PreambleSize = 16; // To preserve alignment

public:
	virtual void* Allocate(std::size_t size) override;
	virtual void Deallocate(void* ptr) override;
	virtual std::size_t GetAllocatedSize(void* ptr) override;
};
