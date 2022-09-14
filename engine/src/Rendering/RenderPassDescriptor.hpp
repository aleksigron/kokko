#pragma once

#include <cstddef>

#include "Core/FixedArray.hpp"
#include "Core/Optional.hpp"

#include "Rendering/RenderTypes.hpp"

namespace kokko
{

struct RenderPassDescriptor
{
    static constexpr size_t MaxColorAttachments = 8;

    FixedArray<RenderPassColorAttachment, MaxColorAttachments> colorAttachments;
    Optional<RenderPassDepthAttachment> depthAttachment;
    Optional<RenderPassStencilAttachment> stencilAttachment;
};

}
