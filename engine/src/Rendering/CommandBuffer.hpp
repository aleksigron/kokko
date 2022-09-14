#pragma once

class Allocator;

namespace kokko
{

class NativeSurface;
class RenderPass;

struct RenderPassDescriptor;

class CommandBuffer
{
public:
    virtual ~CommandBuffer() {}

    virtual RenderPass* CreateRenderPass(const RenderPassDescriptor& descriptor, Allocator* allocator) = 0;
    virtual void Commit() = 0;
    virtual void Present(NativeSurface* surface) = 0;
};

} // namespace kokko
