#pragma once

#include "ObjectId.h"

#include "StringRef.h"

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

struct ShaderProgram
{
private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	bool CompileShader(ShaderType type, const char* path, unsigned& idOut);

public:
	ObjectId id;

	unsigned int oglId;
	int mvpUniformLocation;

	static const unsigned MaxMaterialUniforms = 8;
	unsigned int materialUniformCount;
	ShaderUniform materialUniforms[MaxMaterialUniforms];

	void AddMaterialUniforms(unsigned int count,
							 const ShaderUniformType* types,
							 const StringRef* names);
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
	bool LoadFromConfiguration(const char* configurationPath);
};
