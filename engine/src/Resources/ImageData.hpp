#pragma once

#include <cstddef>

#include "Math/Vec2.hpp"

#include "Rendering/RenderTypes.hpp"

namespace kokko
{

struct ImageData
{
	unsigned char* imageData;
	size_t imageDataSize;

	Vec2i imageSize;
	RenderTextureBaseFormat pixelFormat;
	RenderTextureDataType componentDataType;

	size_t compressedSize;
	bool compressed;

	ImageData();
};

} // namespace kokko
