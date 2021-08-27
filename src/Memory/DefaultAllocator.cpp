#include "Memory/DefaultAllocator.hpp"

#include <cassert>
#include <cstdlib>
#include <memory>

#include "doctest/doctest.h"

#include "Math/Math.hpp"

void* DefaultAllocator::Allocate(std::size_t size, const char* debugTag)
{
	return AllocateAligned(size, DefaultMinAlign, debugTag);
}

void* DefaultAllocator::AllocateAligned(size_t size, size_t alignment, const char* debugTag)
{
	assert(Math::IsPowerOfTwo(alignment));

	if (alignment < DefaultMinAlign)
		alignment = DefaultMinAlign;

	size_t spaceSize = size + alignment - DefaultMinAlign;
	size_t totalSize = spaceSize + MetadataSize;

	void* alloc = std::malloc(totalSize);

	assert(alloc != nullptr);

	if (alloc == nullptr)
		return nullptr;
	
	void* spacePtr = static_cast<char*>(alloc) + MetadataSize;
	void* aligned = std::align(alignment, size, spacePtr, spaceSize);

	assert(aligned != nullptr);

	// Save allocated size
	*(static_cast<size_t*>(aligned) - 2) = size;

	// Save alignment offset
	size_t offset = static_cast<char*>(aligned) - static_cast<char*>(alloc);
	*(static_cast<size_t*>(aligned) - 1) = offset;

	return aligned;
}

void DefaultAllocator::Deallocate(void* ptr)
{
	if (ptr != nullptr)
	{
		size_t offset = *(static_cast<size_t*>(ptr) - 1);
		std::free(static_cast<char*>(ptr) - offset);
	}
}

size_t DefaultAllocator::GetAllocatedSize(void* ptr)
{
	return *(static_cast<size_t*>(ptr) - 2);
}

TEST_CASE("DefaultAllocator.AllocateAligned")
{
	DefaultAllocator allocator;

	for (size_t alignment = 1; alignment <= 256; alignment = alignment << 1)
	{
		void* allocation = allocator.AllocateAligned(1024, alignment);
		intptr_t address = reinterpret_cast<intptr_t>(allocation);
		CHECK(address % alignment == 0);
		allocator.Deallocate(allocation);
	}
}
