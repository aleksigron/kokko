#pragma once

#include "ObjectId.hpp"
#include "BoundingBox.hpp"
#include "VertexFormat.hpp"
#include "BufferRef.hpp"

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
	unsigned int primitiveMode;

	BoundingBox bounds;

	Mesh();
	~Mesh();

	void DeleteBuffers();

	void Upload_3f(BufferRef<Vertex3f> vertices, BufferRef<unsigned short> indices);
	void Upload_3f2f(BufferRef<Vertex3f2f> vertices, BufferRef<unsigned short> indices);
	void Upload_3f3f(BufferRef<Vertex3f3f> vertices, BufferRef<unsigned short> indices);
	void Upload_3f3f2f(BufferRef<Vertex3f3f2f> vertices, BufferRef<unsigned short> indices);
	void Upload_3f3f3f(BufferRef<Vertex3f3f3f> vertices, BufferRef<unsigned short> indices);
};
