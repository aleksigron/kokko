#pragma once

#include "Memory/DefaultAllocator.hpp"

class RootAllocator
{
public:
	static Allocator* GetDefaultAllocator();

	RootAllocator();
	~RootAllocator();

private:
	static Allocator* defaultAllocator;

	static unsigned char buffer[sizeof(DefaultAllocator)];
};
