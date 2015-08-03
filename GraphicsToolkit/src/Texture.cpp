#include "Texture.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "ImageData.h"

void Texture::Upload(const ImageData& image)
{
	glGenTextures(1, &textureGlId);
	glBindTexture(GL_TEXTURE_2D, textureGlId);

	// Upload texture data
	glTexImage2D(GL_TEXTURE_2D, 0, image.pixelFormat, image.size.x, image.size.y,
				 0, image.pixelFormat, GL_UNSIGNED_BYTE, image.data);

	// Set filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}