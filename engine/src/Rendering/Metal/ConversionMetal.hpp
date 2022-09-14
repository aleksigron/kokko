#pragma once

#include "Metal/Metal.hpp"

#include "Math/Vec4.hpp"

#include "Rendering/RenderTypes.hpp"

namespace kokko
{

MTL::LoadAction ConvertLoadAction(AttachmentLoadAction loadAction);
MTL::StoreAction ConvertStoreAction(AttachmentStoreAction storeAction);
MTL::ClearColor ConvertClearColor(const Vec4f& color);

MTL::Texture* ConvertTextureToMetal(TextureHandle texture);
TextureHandle ConvertTextureFromMetal(MTL::Texture* texture);

}
