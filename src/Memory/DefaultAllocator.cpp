#include "Memory/DefaultAllocator.hpp"

#include <cstdlib>

void* DefaultAllocator::Allocate(std::size_t size)
{
	return std::malloc(size);
}

void DefaultAllocator::Deallocate(void* ptr)
{
	std::free(ptr);
}
