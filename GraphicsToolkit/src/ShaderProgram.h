#pragma once

#include <OpenGL/gltypes.h>

struct ShaderProgramId
{
	uint32_t index;
	uint32_t innerId;
};

struct ShaderProgram
{
public:
	ShaderProgramId id;

private:
	enum class ShaderType { Vertex, Fragment };

	bool CompileShader(ShaderType type, const char* filePath, GLuint& shaderIdOut);

public:
	GLuint shaderGlId;
	GLint mvpUniformLocation;
	
	bool Load(const char* vertShaderFilePath, const char* fragShaderFilePath);
};
