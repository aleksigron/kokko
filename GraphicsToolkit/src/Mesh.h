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

	void Upload_PosCol(float* vertexData, unsigned int vertexCount,
					   unsigned short* indices, unsigned int indexCount);

	void UploadVertexData_PosTex(const Buffer<unsigned short>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};
