#pragma once

#include "ObjectId.h"

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

	unsigned int oglId;
	int mvpUniformLocation;
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
};
