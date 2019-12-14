#pragma once

#include "Memory/Allocator.hpp"

class DefaultAllocator : public Allocator
{
public:
	virtual void* Allocate(std::size_t size) override;
	virtual void Deallocate(void* p) override;
};
