#include "Rendering/RenderCommandExecutor.hpp"

#include "Rendering/RenderCommandExecutorOpenGL.hpp"

namespace kokko
{

namespace render
{

CommandExecutor* CommandExecutor::Create(Allocator* allocator)
{
	return allocator->MakeNew<CommandExecutorOpenGL>();
}

}
}
