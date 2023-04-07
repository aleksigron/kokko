#pragma once

class Allocator;

namespace kokko
{

namespace render
{

class CommandBuffer;

class CommandExecutor
{
public:
	static CommandExecutor* Create(Allocator* allocator);

	~CommandExecutor() {}

	virtual void Execute(const CommandBuffer* commandBuffer) = 0;
};

}
}
