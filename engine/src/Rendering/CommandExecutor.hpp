#pragma once

class Allocator;

namespace kokko
{

namespace render
{

struct CommandBuffer;

class CommandExecutor
{
public:
	static CommandExecutor* Create(Allocator* allocator);

	virtual ~CommandExecutor() {}

	virtual void Execute(const CommandBuffer* commandBuffer) = 0;
};

}
}
