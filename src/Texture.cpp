#include "Texture.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "ImageData.hpp"

static int GetFilterModeValue(TextureFilterMode mode)
{
	static const int values[] = {
		GL_NEAREST,
		GL_LINEAR
	};

	return values[static_cast<unsigned long>(mode)];
}

static int GetWrapModeValue(TextureWrapMode mode)
{
	static const int values[] = {
		GL_REPEAT,
		GL_MIRRORED_REPEAT,
		GL_CLAMP_TO_EDGE,
		GL_CLAMP_TO_BORDER
	};

	return values[static_cast<unsigned long>(mode)];
}

void Texture::Upload(const ImageData& image)
{
	this->Upload(image, TextureOptions());
}

void Texture::Upload(const ImageData& image, const TextureOptions& options)
{
	textureSize.x = static_cast<float>(image.imageSize.x);
	textureSize.y = static_cast<float>(image.imageSize.y);

	targetType = GL_TEXTURE_2D;

	glGenTextures(1, &driverId);
	glBindTexture(targetType, driverId);

	if (image.compressed)
	{
		// Upload compressed texture data
		glCompressedTexImage2D(targetType, 0, image.pixelFormat,
							   image.imageSize.x, image.imageSize.y, 0,
							   image.compressedSize, image.imageData);
	}
	else
	{
		// Upload uncompressed texture data
		glTexImage2D(targetType, 0, image.pixelFormat,
					 image.imageSize.x, image.imageSize.y, 0,
					 image.pixelFormat, image.componentDataType, image.imageData);
	}

	int filterMode = GetFilterModeValue(options.filter);

	// Set filter
	glTexParameteri(targetType, GL_TEXTURE_MIN_FILTER, filterMode);
	glTexParameteri(targetType, GL_TEXTURE_MAG_FILTER, filterMode);

	int wrapMode = GetWrapModeValue(options.wrap);

	glTexParameteri(targetType, GL_TEXTURE_WRAP_R, wrapMode);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}
