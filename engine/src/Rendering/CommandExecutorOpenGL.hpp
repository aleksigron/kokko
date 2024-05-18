#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/Queue.hpp"
#include "Core/StringView.hpp"

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
	CommandExecutorOpenGL(Allocator* allocator);

	void Execute(const CommandBuffer* commandBuffer) override;

private:
	size_t ParseCommand(CommandType type, const uint8_t* commandBegin);

	const CommandBuffer* cmdBuffer;

	Array<ConstStringView> debugScopeStack;
	Queue<CommandType> commandHistory;
};

}
}
