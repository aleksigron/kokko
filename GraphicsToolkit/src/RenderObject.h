#pragma once

#include <OpenGL/gltypes.h>

#include "Transform.h"
#include "Buffer.h"
#include "VertexFormat.h"
#include "ObjectId.h"

struct RenderObject
{
	ObjectId id;

	Transform transform;

	ObjectId shader;
	ObjectId texture;
	
	GLuint vertexArrayObject;
	
	GLsizei indexCount;
	GLenum indexElementType;

	bool hasTexture;

	void UploadVertexData_PosCol(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosCol>& vertex);
	
	void UploadVertexData_PosTex(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};