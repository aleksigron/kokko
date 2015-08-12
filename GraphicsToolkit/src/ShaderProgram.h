#pragma once

#include "ObjectId.h"

#include "ShaderUniform.h"

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
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
	bool LoadFromConfiguration(const char* configurationPath);
};
