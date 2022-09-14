#pragma once

#include "Rendering/RenderPass.hpp"

namespace MTL
{
class CommandBuffer;
class RenderCommandEncoder;
}

namespace kokko
{

struct RenderPassDescriptor;

class RenderPassMetal : public RenderPass
{
public:
    RenderPassMetal(MTL::CommandBuffer* commandBuffer, const RenderPassDescriptor& descriptor);
    ~RenderPassMetal();

private:
    MTL::RenderCommandEncoder* encoder;
};

}
