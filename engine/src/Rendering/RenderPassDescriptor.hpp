#pragma once

#include <cstddef>

#include "Core/FixedArray.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

namespace kokko
{

struct RenderPassDescriptor
{
    static constexpr size_t MaxColorAttachments = 8;

    FixedArray<RenderPassColorAttachment, MaxColorAttachments> colorAttachments;
    RenderPassDepthAttachment depthAttachment;
    RenderPassStencilAttachment stencilAttachment;
};

}
