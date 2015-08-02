#pragma once

#include <OpenGL/gltypes.h>

#include "ObjectId.h"
#include "Vec2.h"

struct ImageData;

struct Texture
{
	ObjectId id;

	GLuint textureGlId;

	void Upload(const ImageData& image);
};