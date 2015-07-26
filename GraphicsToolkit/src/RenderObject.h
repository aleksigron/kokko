#pragma once

#include <cstdint>

#include <OpenGL/gltypes.h>

#include "Transform.h"
#include "Buffer.h"
#include "VertexFormat.h"

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
	
	GLsizei indexCount;
	GLenum indexElementType;
	
	GLuint shaderProgram;
	
	void UploadVertexData_PosCol(const Buffer<uint16_t>& index, const Buffer<Vertex_PosCol>& vertex);
};