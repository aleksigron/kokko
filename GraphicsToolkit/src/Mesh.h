#pragma once

#include "ObjectId.h"
#include "Buffer.h"
#include "VertexFormat.h"

struct Mesh
{
private:
	void CreateBuffers(void* vertexBuffer, unsigned int vertexBufferSize,
						   void* indexBuffer, unsigned int indexBufferSize);

public:
	ObjectId id;

	unsigned int vertexArrayObject;

	int indexCount;
	unsigned int indexElementType;

	void Upload_PosCol(float* vertexData, unsigned int vertexCount,
					   unsigned short* indexData, unsigned int indexCount);

	void Upload_PosNorCol(float* vertexData, unsigned int vertexCount,
						  unsigned short* indexData, unsigned int indexCount);

	void UploadVertexData_PosTex(const Buffer<unsigned short>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};
