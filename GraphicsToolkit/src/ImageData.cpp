#include "ImageData.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

void ImageData::LoadTestData()
{
	size.x = 64;
	size.y = 64;

	int pixels = size.x * size.y;
	dataSize = pixels * 4;
	hasAlpha = true;
	data = new unsigned char[dataSize];

	for (int i = 0; i < pixels; ++i)
	{
		*(data + i*4 + 0) = (i * 3 + 91) % 251;
		*(data + i*4 + 1) = (i * 7 + 59) % 239;
		*(data + i*4 + 2) = (i * 5 + 13) % 251;
		*(data + i*4 + 3) = 255;
	}
}

bool ImageData::LoadPng(const char* filePath)
{
	int components = 0;
	data = stbi_load(filePath, &size.x, &size.y, &components, 0);

	if (data != nullptr)
	{
		hasAlpha = components == 4;

		return true;
	}
	else
		return false;
}

void ImageData::DeallocateData()
{
	if (data != nullptr)
		stbi_image_free(data);
}