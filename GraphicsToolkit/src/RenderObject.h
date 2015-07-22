#pragma once

#include <cstdint>

#include <OpenGL/gltypes.h>

#include "Transform.h"
#include "ShaderProgram.h"

struct RenderObjectId
{
	uint32_t index;
	uint32_t innerId;
};

struct RenderObject
{
	RenderObjectId id;
	
	Transform transform;
	
	GLuint vertexArrayObject;
	GLuint vertexPositionBuffer;
	GLuint vertexColorBuffer;
	
	GLsizei vertexCount;
	
	ShaderProgram shader;
	
//		if (this->vertexBufferObject != 0)
//			glDeleteBuffers(1, &this->vertexBufferObject);
//		
//		if (this->vertexArrayObject != 0)
//			glDeleteVertexArrays(1, &this->vertexArrayObject);
};