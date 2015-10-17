#pragma once

#include "ObjectId.h"
#include "Vec2.h"

struct ImageData;

struct Texture
{
	ObjectId id;
	Vec2i textureSize;
	unsigned int textureGlId;

	void Upload(const ImageData& image);
};