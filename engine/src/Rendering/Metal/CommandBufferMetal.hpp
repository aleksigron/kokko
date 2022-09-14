#pragma once

#include "Metal/Metal.hpp"

#include "Rendering/CommandBuffer.hpp"

class Allocator;

namespace kokko
{

class RenderDeviceMetal;

class CommandBufferMetal : public CommandBuffer
{
public:
    CommandBufferMetal(RenderDeviceMetal* renderDevice, MTL::CommandBuffer* commandBuffer);
    ~CommandBufferMetal();

    virtual RenderPass* CreateRenderPass(const RenderPassDescriptor& descriptor, Allocator* allocator) override;
    virtual void Commit() override;
    virtual void Present(NativeSurface* surface) override;

private:
    MTL::CommandBuffer* buffer;
};

}
