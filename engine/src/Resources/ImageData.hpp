#pragma once

#include <cstddef>

#include "Math/Vec2.hpp"

struct ImageData
{
	unsigned char* imageData;
	size_t imageDataSize;

	Vec2i imageSize;
	unsigned int pixelFormat;
	unsigned int componentDataType;

	size_t compressedSize;
	bool compressed;

	ImageData();
};
