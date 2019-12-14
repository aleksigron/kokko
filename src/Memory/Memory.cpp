#include "Memory/Memory.hpp"

#include "Memory/DefaultAllocator.hpp"

unsigned char staticBuffer[sizeof(DefaultAllocator)];
Allocator* defaultAllocator = nullptr;

namespace Memory
{

void InitializeMemorySystem()
{
	defaultAllocator = new (staticBuffer) DefaultAllocator();
}

void DeinitializeMemorySystem()
{

}

Allocator* GetDefaultAllocator()
{
	return defaultAllocator;
}

}
