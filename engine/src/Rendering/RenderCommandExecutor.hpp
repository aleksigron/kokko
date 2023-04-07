#pragma once

namespace kokko
{

namespace render
{

class CommandBuffer;

class CommandExecutor
{
public:
	virtual void Execute(const CommandBuffer* commandBuffer) = 0;
};

}
}
