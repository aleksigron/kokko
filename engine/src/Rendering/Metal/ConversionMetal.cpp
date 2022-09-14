#include "Rendering/Metal/ConversionMetal.hpp"

#include <cassert>

#include "Metal/Metal.hpp"

namespace kokko
{

MTL::LoadAction ConvertLoadAction(AttachmentLoadAction loadAction)
{
    switch (loadAction)
    {
    case AttachmentLoadAction::DontCare:
        return MTL::LoadActionDontCare;
    case AttachmentLoadAction::Clear:
        return MTL::LoadActionClear;
    case AttachmentLoadAction::Load:
        return MTL::LoadActionLoad;
    }

    assert(false);
    return MTL::LoadActionLoad;
}

MTL::StoreAction ConvertStoreAction(AttachmentStoreAction storeAction)
{
    switch (storeAction)
    {
    case AttachmentStoreAction::DontCare:
        return MTL::StoreActionDontCare;
    case AttachmentStoreAction::Store:
        return MTL::StoreActionStore;
    }

    assert(false);
    return MTL::StoreActionStore;
}

MTL::ClearColor ConvertClearColor(const Vec4f& color)
{
    return MTL::ClearColor::Make(color.x, color.y, color.z, color.w);
}

MTL::Texture* ConvertTextureToMetal(TextureHandle texture)
{
    return reinterpret_cast<MTL::Texture*>(texture.storage);
}

TextureHandle ConvertTextureFromMetal(MTL::Texture* texture)
{

}

} // namespace kokko
