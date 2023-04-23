#pragma once

#include <cstddef>
#include <cstdint>

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"

#include "Resources/ShaderId.hpp"

struct MeshDrawData;

namespace kokko
{

struct PostProcessRenderPass
{
	static const size_t MaxTextureCount = 10;

	uint32_t textureNameHashes[MaxTextureCount];
	render::TextureId textureIds[MaxTextureCount];
	render::SamplerId samplerIds[MaxTextureCount];
	unsigned int textureCount;

	render::BufferId uniformBufferId;
	uint32_t uniformBindingPoint;
	uint32_t uniformBufferRangeStart;
	uint32_t uniformBufferRangeSize;

	render::FramebufferId framebufferId;
	Vec2i viewportSize;

	ShaderId shaderId;

	bool enableBlending;
	RenderBlendFactor sourceBlendFactor;
	RenderBlendFactor destinationBlendFactor;
};

} // namespace kokko
