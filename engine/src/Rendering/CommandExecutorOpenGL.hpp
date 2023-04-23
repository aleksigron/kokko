#pragma once

#include <cstdint>

#include "Core/Array.hpp"

#include "Rendering/RenderCommand.hpp"
#include "Rendering/CommandExecutor.hpp"

class Allocator;

namespace kokko
{

namespace render
{

class CommandExecutorOpenGL : public CommandExecutor
{
public:
	CommandExecutorOpenGL();

	void Execute(const CommandBuffer* commandBuffer) override;

private:
	size_t ParseCommand(CommandType type, const uint8_t* commandBegin);

	const CommandBuffer* cmdBuffer;
};

}
}
