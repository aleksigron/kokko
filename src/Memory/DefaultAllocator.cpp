#include "Memory/DefaultAllocator.hpp"

#include <cstdlib>

void* DefaultAllocator::Allocate(std::size_t size, const char* debugTag)
{
	void* ptr = std::malloc(size + PreambleSize);
	
	if (ptr == nullptr)
		return nullptr;

	*static_cast<std::size_t*>(ptr) = size; // Save allocated size to preamble
	return static_cast<char*>(ptr) + PreambleSize;
}

void DefaultAllocator::Deallocate(void* ptr)
{
	if (ptr != nullptr)
	{
		std::free(static_cast<char*>(ptr) - PreambleSize);
	}
}

std::size_t DefaultAllocator::GetAllocatedSize(void* ptr)
{
	return *reinterpret_cast<std::size_t*>(static_cast<char*>(ptr) - PreambleSize);
}
