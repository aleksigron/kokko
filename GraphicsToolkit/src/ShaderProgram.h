#pragma once

#include <OpenGL/gltypes.h>

#include "ObjectId.h"

struct ShaderProgram
{
public:
	ObjectId id;

private:
	enum class ShaderType { Vertex, Fragment };

	bool CompileShader(ShaderType type, const char* filePath, GLuint& shaderIdOut);

public:
	GLuint shaderGlId;
	GLint mvpUniformLocation;
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
};
