#include "ImageData.h"

#include <OpenGL/gl3.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#pragma clang diagnostic ignored "-Wunused-function"
#pragma clang diagnostic ignored "-Wdisabled-macro-expansion"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb/stb_image.h"
#pragma clang diagnostic pop

bool ImageData::LoadPng(const char* filePath)
{
	int components = 0;
	data = stbi_load(filePath, &size.x, &size.y, &components, 0);

	if (data != nullptr)
	{
		assert(components >= 0 && components < 4);

		static const GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		pixelFormat = formats[components - 1];

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