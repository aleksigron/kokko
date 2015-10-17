#pragma once

#include "ObjectId.hpp"
#include "Vec2.hpp"

struct ImageData;

struct Texture
{
	ObjectId id;
	Vec2i textureSize;
	unsigned int textureGlId;

	void Upload(const ImageData& image);
};