#pragma once

#include "Rendering/CommandBuffer.hpp"

class Allocator;

namespace MTL
{
class CommandBuffer;
class CommandQueue;
}

namespace kokko
{

class CommandBufferMetal : public CommandBuffer
{
public:
    explicit CommandBufferMetal(MTL::CommandQueue* queue);
    ~CommandBufferMetal();

    virtual RenderPass* CreateRenderPass(const RenderPassDescriptor& descriptor, Allocator* allocator) override;
    virtual void Commit() override;
    virtual void Present(NativeSurface* surface) override;

private:
    MTL::CommandBuffer* buffer;
};

}
