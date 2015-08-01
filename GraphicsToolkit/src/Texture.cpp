#include "Texture.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "ImageData.h"

void Texture::Upload(const ImageData& image)
{
	GLenum format = image.hasAlpha ? GL_RGBA : GL_RGB;

	glGenTextures(1, &textureGlId);
	glBindTexture(GL_TEXTURE_2D, textureGlId);

	// Upload texture data
	glTexImage2D(GL_TEXTURE_2D, 0, format, image.size.x, image.size.y,
				 0, format, GL_UNSIGNED_BYTE, image.data);

	// Set filter
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// Unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);
}