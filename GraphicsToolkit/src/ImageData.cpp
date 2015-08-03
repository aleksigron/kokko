#include "ImageData.h"

#include <OpenGL/gl3.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"

bool ImageData::LoadPng(const char* filePath)
{
	int components = 0;
	data = stbi_load(filePath, &size.x, &size.y, &components, 0);

	if (data != nullptr)
	{
		pixelFormat = (components == 4 ? GL_RGBA : GL_RGB);
		return true;
	}
	else
	{
		assert(false);
		return false;
	}
}

void ImageData::DeallocateData()
{
	if (data != nullptr)
		stbi_image_free(data);
}