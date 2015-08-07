#pragma once

enum class ShaderUniformType
{
	Texture2D,
	Mat4x4,
	Vec3,
	Vec2,
	Float
};

struct ShaderUniform
{
	int location;
	ShaderUniformType type;
};

struct ShaderMaterialUniform : ShaderUniform
{
	uint16_t dataOffset;
};