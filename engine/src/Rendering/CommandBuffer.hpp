#pragma once

class Allocator;

namespace kokko
{

class RenderPass;

struct RenderPassDescriptor;

class CommandBuffer
{
public:
    virtual ~CommandBuffer() {}

    virtual RenderPass* CreateRenderPass(const RenderPassDescriptor& descriptor, Allocator* allocator) = 0;
    virtual void Commit() = 0;
    virtual void Present() = 0;
};

} // namespace kokko
