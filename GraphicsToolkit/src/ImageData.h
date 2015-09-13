#pragma once

#include "Vec2.h"
#include "Buffer.h"

struct ImageData
{
	Buffer<unsigned char> dataBuffer;

	unsigned char* imageData = nullptr;
	unsigned long imageDataSize = 0;

	Vec2i imageSize;
	unsigned int pixelFormat;
	unsigned int componentDataType;

	unsigned int compressedSize;
	bool compressed;

	bool LoadGlraw(const char* filePath);
};