#pragma once

#include "ObjectId.hpp"
#include "BoundingBox.hpp"

struct Mesh
{
private:
	enum BufferType { VertexBuffer, IndexBuffer };

	void CreateBuffers(void* vertexBuffer, unsigned int vertexBufferSize,
					   void* indexBuffer, unsigned int indexBufferSize);

public:
	ObjectId id;

	unsigned int vertexArrayObject;
	unsigned int bufferObjects[2];

	int indexCount;
	unsigned int indexElementType;

	BoundingBox bounds;

	void DeleteBuffers();

	void Upload_3f2f(float* vertexData, unsigned int vertexCount,
					 unsigned short* indexData, unsigned int indexCount);
	
	void Upload_3f3f(float* vertexData, unsigned int vertexCount,
					   unsigned short* indexData, unsigned int indexCount);

	void Upload_3f3f2f(float* vertexData, unsigned int vertexCount,
						  unsigned short* indexData, unsigned int indexCount);

	void Upload_3f3f3f(float* vertexData, unsigned int vertexCount,
						  unsigned short* indexData, unsigned int indexCount);
};
