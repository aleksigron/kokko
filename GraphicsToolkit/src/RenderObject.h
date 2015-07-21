#pragma once

#include <OpenGL/gltypes.h>

#include "Transform.h"
#include "ShaderProgram.h"
#include "Buffer.h"
#include "Vec3.h"

class RenderObject : public Transform
{
private:
	GLuint vertexArrayObject = 0;
	GLuint vertexBufferObject = 0;
	
	GLsizei vertexCount = 0;
	
	ShaderProgram shader;
	
public:
	RenderObject();
	~RenderObject();
	
	void SetVertexBufferData(Buffer<Vec3f>& vertexBuffer);
	
	inline GLuint GetVertexBuffer() const { return this->vertexBufferObject; }
	
	inline const ShaderProgram* GetShaderProgram() const { return &this->shader; }
	inline GLuint GetShaderProgramID() const { return this->shader.GetID(); }
	
	GLsizei GetVertexCount() const { return this->vertexCount; }
};