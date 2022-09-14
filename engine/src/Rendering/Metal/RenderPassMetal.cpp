#include "Rendering/Metal/RenderPassMetal.hpp"

#include "Metal/Metal.hpp"

#include "Rendering/RenderPassDescriptor.hpp"
#include "Rendering/Metal/ConversionMetal.hpp"

namespace kokko
{

RenderPassMetal::RenderPassMetal(MTL::CommandBuffer* commandBuffer, const RenderPassDescriptor& descriptor) :
    encoder(nullptr)
{
    auto desc = NS::TransferPtr(MTL::RenderPassDescriptor::alloc()->init());

    if (descriptor.colorAttachments.GetCount() != 0)
    {
        auto color = NS::TransferPtr(MTL::RenderPassColorAttachmentDescriptor::alloc()->init());

        for (size_t i = 0, count = descriptor.colorAttachments.GetCount(); i < count; ++i)
        {
            const RenderPassColorAttachment& src = descriptor.colorAttachments[i];

            color->setLoadAction(ConvertLoadAction(src.loadAction));
            color->setStoreAction(ConvertStoreAction(src.storeAction));
            color->setClearColor(ConvertClearColor(src.clearColor));
            color->setTexture(ConvertTextureToMetal(src.texture));

            desc->colorAttachments()->setObject(color.get(), i);
        }
    }

    if (descriptor.depthAttachment.HasValue())
    {
        auto depth = NS::TransferPtr(MTL::RenderPassDepthAttachmentDescriptor::alloc()->init());
        const RenderPassDepthAttachment& src = descriptor.depthAttachment.GetValue();

        depth->setLoadAction(ConvertLoadAction(src.loadAction));
        depth->setStoreAction(ConvertStoreAction(src.storeAction));
        depth->setClearDepth(src.clearDepth);
        depth->setTexture(ConvertTextureToMetal(src.texture));

        desc->setDepthAttachment(depth.get());
    }

    if (descriptor.depthAttachment.HasValue())
    {
        auto stencil = NS::TransferPtr(MTL::RenderPassStencilAttachmentDescriptor::alloc()->init());
        const RenderPassStencilAttachment& src = descriptor.stencilAttachment.GetValue();

        stencil->setLoadAction(ConvertLoadAction(src.loadAction));
        stencil->setStoreAction(ConvertStoreAction(src.storeAction));
        stencil->setClearStencil(src.clearStencil);
        stencil->setTexture(ConvertTextureToMetal(src.texture));

        desc->setStencilAttachment(stencil.get());
    }

    encoder = commandBuffer->renderCommandEncoder(desc.get());
    encoder->retain();
}

RenderPassMetal::~RenderPassMetal()
{
    encoder->endEncoding();
    encoder->release();
}

}
