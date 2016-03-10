#pragma once

#include "ObjectId.hpp"

#include "StringRef.hpp"

class StackAllocator;

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

	static const unsigned int TypeCount = 5;
	static const char* const TypeNames[TypeCount];
};

struct ShaderProgram
{
private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	bool CompileShader(ShaderType type, const char* path, unsigned& idOut);

	StackAllocator* allocator;

public:
	ObjectId id;

	unsigned int oglId;
	int mvpUniformLocation;

	static const unsigned MaxMaterialUniforms = 8;
	unsigned int materialUniformCount;
	ShaderUniform materialUniforms[MaxMaterialUniforms];

	void SetAllocator(StackAllocator* allocator);

	void AddMaterialUniforms(unsigned int count,
							 const ShaderUniformType* types,
							 const StringRef* names);
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
	bool LoadFromConfiguration(const char* configurationPath);
};
