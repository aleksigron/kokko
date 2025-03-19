#include "Memory/RootAllocator.hpp"

#include <cassert>

namespace kokko
{

Allocator* RootAllocator::defaultAllocator = nullptr;
unsigned char RootAllocator::buffer[sizeof(DefaultAllocator)];

RootAllocator::RootAllocator()
{
	assert(defaultAllocator == nullptr);

	if (defaultAllocator == nullptr)
	{
		defaultAllocator = new (buffer) DefaultAllocator();
	}
}

RootAllocator::~RootAllocator()
{
	assert(defaultAllocator != nullptr);

	defaultAllocator->~Allocator();
	defaultAllocator = nullptr;
}

Allocator* RootAllocator::GetDefaultAllocator()
{
	return defaultAllocator;
}

} // namespace kokko
