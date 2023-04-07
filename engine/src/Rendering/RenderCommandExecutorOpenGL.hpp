#pragma once

#include <cstdint>

#include "Core/Array.hpp"

#include "Rendering/RenderCommand.hpp"
#include "Rendering/RenderCommandExecutor.hpp"

class Allocator;

namespace kokko
{

namespace render
{

class CommandExecutorOpenGL : public CommandExecutor
{
public:
	CommandExecutorOpenGL(Allocator* allocator);

	void Execute(const CommandBuffer* commandBuffer) override;

private:
	size_t ParseCommand(RenderCommandType type, const uint8_t* commandBegin);

	const CommandBuffer* cmdBuffer;

	Array<uint32_t> tempIds;
};

}
}
