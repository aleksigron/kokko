#pragma once

#include <cstdint>

#include "Vec2.hpp"

struct ImageData;

struct Texture
{
	uint32_t nameHash;
	Vec2i textureSize;
	unsigned int driverId;

	void Upload(const ImageData& image);
};
