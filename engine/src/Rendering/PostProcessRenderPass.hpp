#pragma once

#include <cstddef>
#include <cstdint>

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"

#include "Resources/ShaderId.hpp"

struct MeshDrawData;

struct PostProcessRenderPass
{
	static const size_t MaxTextureCount = 10;

	uint32_t textureNameHashes[MaxTextureCount];
	kokko::RenderTextureId textureIds[MaxTextureCount];
	kokko::RenderSamplerId samplerIds[MaxTextureCount];
	unsigned int textureCount;

	kokko::RenderBufferId uniformBufferId;
	uint32_t uniformBindingPoint;
	uint32_t uniformBufferRangeStart;
	uint32_t uniformBufferRangeSize;

	kokko::RenderFramebufferId framebufferId;
	Vec2i viewportSize;

	ShaderId shaderId;

	bool enableBlending;
	RenderBlendFactor sourceBlendFactor;
	RenderBlendFactor destinationBlendFactor;
};
