#include "Rendering/Metal/CommandBufferMetal.hpp"

#include "Metal/Metal.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/RenderPassDescriptor.hpp"
#include "Rendering/Metal/RenderPassMetal.hpp"

namespace kokko
{

CommandBufferMetal::CommandBufferMetal(MTL::CommandQueue* queue)
{
    buffer = queue->commandBuffer();
    buffer->retain();
}

CommandBufferMetal::~CommandBufferMetal()
{
    buffer->release();
}

RenderPass* CommandBufferMetal::CreateRenderPass(const RenderPassDescriptor& descriptor, Allocator* allocator)
{
    return allocator->MakeNew<RenderPassMetal>(buffer, descriptor);
}

void CommandBufferMetal::Commit()
{
    buffer->commit();
}

void CommandBufferMetal::Present(NativeSurface* surface)
{
    void* ptr = surface;
    MTL::Drawable* drawable = static_cast<MTL::Drawable*>(ptr);
    buffer->presentDrawable(drawable);
}

} // namespace kokko
