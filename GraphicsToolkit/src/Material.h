#pragma once

#include "ObjectId.h"

struct Material
{
	static const unsigned MaxTextures = 4;

	ObjectId id;

	ObjectId shader;

	ObjectId textures[MaxTextures];
};

inline bool operator < (const Material& lhs, const Material& rhs)
{
	return lhs.shader < rhs.shader;
}