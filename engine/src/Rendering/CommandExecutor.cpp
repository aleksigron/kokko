#include "Rendering/CommandExecutor.hpp"

#include "Rendering/CommandExecutorOpenGL.hpp"

namespace kokko
{

namespace render
{

CommandExecutor* CommandExecutor::Create(Allocator* allocator)
{
	return allocator->MakeNew<CommandExecutorOpenGL>(allocator);
}

}
}
