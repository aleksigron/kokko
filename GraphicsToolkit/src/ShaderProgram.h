#pragma once

#include "ImmutableString.h"
#include "ObjectId.h"

struct ShaderUniform
{
	enum class Type
	{
		Texture2D,
		Mat4x4,
		Vec3,
		Vec2,
		Float
	};

	ImmutableString name;
	int layoutLocation;
	Type type;
};

struct ShaderProgram
{
private:
	enum class ShaderType
	{
		Vertex,
		Fragment
	};

	bool CompileShader(ShaderType type, const char* filePath, unsigned int& idOut);

public:
	ObjectId id;

	unsigned oglId;
	int mvpUniformLocation;
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
};
