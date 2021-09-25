#pragma once

#include "Memory/Allocator.hpp"

namespace Memory
{
	void InitializeMemorySystem();
	void DeinitializeMemorySystem();

	Allocator* GetDefaultAllocator();
}
