#pragma once

#include "ObjectId.h"
#include "Buffer.h"
#include "VertexFormat.h"

struct Mesh
{
	ObjectId id;

	unsigned int vertexArrayObject;

	int indexCount;
	unsigned int indexElementType;

	void UploadVertexData_PosCol(const Buffer<unsigned short>& index,
								 const Buffer<Vertex_PosCol>& vertex);

	void UploadVertexData_PosTex(const Buffer<unsigned short>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};
