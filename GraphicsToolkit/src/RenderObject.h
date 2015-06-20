#pragma once

#include <OpenGL/gltypes.h>

#include "Buffer.h"
#include "Vec3.h"

class RenderObject
{
private:
	GLuint vertexArrayObject = 0;
	GLuint vertexBufferObject = 0;
	GLuint shaderProgram = 0;
	
public:
	RenderObject();
	~RenderObject();
	
	void SetVertexBufferData(Buffer<Vec3f>& vertexBuffer);
	
	GLuint GetVertexBuffer() const { return this->vertexBufferObject; }
	GLuint GetShaderProgram() const { return this->shaderProgram; }
};