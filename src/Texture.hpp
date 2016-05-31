#pragma once

#include <cstdint>

#include "Vec2.hpp"

struct ImageData;

struct Texture
{
	enum class FilteringMode
	{
		Nearest,
		Linear
	};

	uint32_t nameHash;
	Vec2i textureSize;
	unsigned int driverId;

	void Upload(const ImageData& image, FilteringMode mode);
};
