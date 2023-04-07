#pragma once

#include <cstdint>

#include "Core/Array.hpp"

namespace kokko
{

namespace render
{

struct CommandBuffer
{
	explicit CommandBuffer(Allocator* allocator) :
		commands(allocator),
		commandData(allocator),
		commandResources(allocator)
	{
	}

	Array<uint8_t> commands;
	Array<uint8_t> commandData;
	Array<void*> commandResources;
};

}
}
