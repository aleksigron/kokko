#pragma once

#include <cstdint>

#include "ObjectId.h"
#include "ShaderUniform.h"

struct Material
{
	// sizeof(ShaderMaterialUniform) is likely 8, so 8 nicely fit in 64 bytes
	static const unsigned MaxUniformCount = 8;

	ObjectId id;

	ObjectId shader;

	int mvpUniformLocation;

	unsigned int materialUniformCount;
	ShaderMaterialUniform materialUniforms[MaxUniformCount];

	unsigned char* uniformData;
};

inline bool operator < (const Material& lhs, const Material& rhs)
{
	return lhs.shader < rhs.shader;
}