#pragma once

#include <OpenGL/gltypes.h>

#include "Vec2.h"

struct ImageData;

struct TextureId
{
	uint32_t index;
	uint32_t innerId;
};

struct Texture
{
	TextureId id;

	GLuint textureGlId;

	void Upload(const ImageData& image);
};