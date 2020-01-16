#pragma once

#include "Math/Vec2.hpp"
#include "BufferRef.hpp"

struct ImageData
{
	unsigned char* imageData;
	unsigned long imageDataSize;

	Vec2i imageSize;
	unsigned int pixelFormat;
	unsigned int componentDataType;

	unsigned int compressedSize;
	bool compressed;

	ImageData();

	bool LoadGlraw(BufferRef<unsigned char> fileContent);
};
