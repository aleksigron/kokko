#pragma once

#include <cstdint>

#include "BufferRef.hpp"
#include "Rendering/TransparencyType.hpp"

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

	bool Compile(ShaderType type, BufferRef<char> source, unsigned& idOut);
	bool CompileAndLink(BufferRef<char> vertSource, BufferRef<char> fragSource);
	void AddMaterialUniforms(unsigned int count,
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
	
	bool LoadFromConfiguration(BufferRef<char> configuration);
};
