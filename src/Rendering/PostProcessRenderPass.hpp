#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

#include "Resources/ShaderId.hpp"

struct MeshDrawData;

struct PostProcessRenderPass
{
	static const size_t MaxTextureCount = 10;

	uint32_t textureNameHashes[MaxTextureCount];
	unsigned int textureIds[MaxTextureCount];
	unsigned int samplerIds[MaxTextureCount];
	unsigned int textureCount;

	unsigned int uniformBufferId;
	unsigned int uniformBindingPoint;
	unsigned int uniformBufferRangeStart;
	unsigned int uniformBufferRangeSize;

	unsigned int framebufferId;
	Vec2i viewportSize;

	ShaderId shaderId;

	bool enableBlending;
	RenderBlendFactor sourceBlendFactor;
	RenderBlendFactor destinationBlendFactor;
};
