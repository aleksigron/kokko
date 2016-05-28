#pragma once

#include "Vec2.hpp"
#include "Buffer.hpp"

struct ImageData
{
	Buffer<unsigned char> dataBuffer;

	unsigned char* imageData;
	unsigned long imageDataSize;

	Vec2i imageSize;
	unsigned int pixelFormat;
	unsigned int componentDataType;

	unsigned int compressedSize;
	bool compressed;

	ImageData();

	bool LoadGlraw(const char* filePath);
};
