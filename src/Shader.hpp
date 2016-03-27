#pragma once

#include <cstdint>

#include "Buffer.hpp"

struct StringRef;
class StackAllocator;

enum class ShaderUniformType
{
	Tex2D,
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

	static const unsigned int TypeCount = 7;
	static const char* const TypeNames[TypeCount];
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

	bool Compile(ShaderType type, Buffer<char>& source, unsigned& idOut);

	StackAllocator* allocator;

public:
	uint32_t nameHash;

	unsigned int driverId;

	int uniformMVP;
	int uniformMV;

	static const unsigned MaxMaterialUniforms = 8;

	unsigned int materialUniformCount;
	ShaderUniform materialUniforms[MaxMaterialUniforms];

	void SetAllocator(StackAllocator* allocator);

	void AddMaterialUniforms(unsigned int count,
							 const ShaderUniformType* types,
							 const StringRef* names);
	
	bool CompileAndLink(Buffer<char>& vertexSource, Buffer<char>& fragmentSource);
	
	bool LoadFromConfiguration(Buffer<char>& configuration);
};
