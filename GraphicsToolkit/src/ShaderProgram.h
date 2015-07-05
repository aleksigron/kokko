#pragma once

#include <OpenGL/gltypes.h>

class ShaderProgram
{
private:
	enum class ShaderType { Vertex, Fragment };
	
	GLuint shaderProgram = 0;
	
	bool CompileShader(ShaderType type, const char* filePath, GLuint& shaderIdOut);
	
public:
	ShaderProgram();
	~ShaderProgram();
	
	inline GLuint GetID() const { return this->shaderProgram; }
	
	bool LoadShaders(const char* vertShaderFilePath, const char* fragShaderFilePath);
};





