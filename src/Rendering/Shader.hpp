#pragma once

#include <cstdint>

#include "Core/BufferRef.hpp"
#include "Rendering/TransparencyType.hpp"

class Allocator;
class RenderDevice;

enum class ShaderUniformType
{
	Tex2D,
	TexCube,
	Mat4x4,
	Vec4,
	Vec3,
	Vec2,
	Float,
	Int
};

struct ShaderUniform
{
	int location;
	uint32_t nameHash;
	ShaderUniformType type;

	static const unsigned int TypeCount = 8;
	static const unsigned int TypeSizes[TypeCount];
};

struct Shader
{
	uint32_t nameHash;

	unsigned int driverId;

	int uniformMatMVP;
	int uniformMatMV;
	int uniformMatVP;
	int uniformMatM;
	int uniformMatV;
	int uniformMatP;

	TransparencyType transparencyType;

	static const unsigned MaxMaterialUniforms = 8;

	unsigned int materialUniformCount;
	ShaderUniform materialUniforms[MaxMaterialUniforms];

	Shader();
};
