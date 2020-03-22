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
private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	bool CompileAndLink(
		BufferRef<char> vertSource,
		BufferRef<char> fragSource,
		Allocator* allocator,
		RenderDevice* renderDevice);

	bool Compile(
		Allocator* allocator,
		RenderDevice* renderDevice,
		ShaderType type,
		BufferRef<char> source,
		unsigned int& shaderIdOut);

	void AddMaterialUniforms(
		RenderDevice* renderDevice,
		unsigned int count,
		const ShaderUniformType* types,
		const char** names);

public:
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
	
	bool LoadFromConfiguration(
		BufferRef<char> configuration,
		Allocator* allocator,
		RenderDevice* renderDevice);
};
