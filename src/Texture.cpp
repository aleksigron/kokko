#include "Texture.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "ImageData.hpp"

static const int textureFilteringModeValues[] = { GL_NEAREST, GL_LINEAR };

void Texture::Upload(const ImageData& image, FilteringMode mode)
{
	textureSize = image.imageSize;

	glGenTextures(1, &driverId);
	glBindTexture(GL_TEXTURE_2D, driverId);

	if (image.compressed)
	{
		// Upload compressed texture data
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, image.pixelFormat,
							   image.imageSize.x, image.imageSize.y, 0,
							   image.compressedSize, image.imageData);
	}
	else
	{
		// Upload uncompressed texture data
		glTexImage2D(GL_TEXTURE_2D, 0, image.pixelFormat,
					 image.imageSize.x, image.imageSize.y, 0,
					 image.pixelFormat, image.componentDataType, image.imageData);
	}

	int filteringMode = textureFilteringModeValues[static_cast<int>(mode)];

	// Set filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filteringMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filteringMode);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}
