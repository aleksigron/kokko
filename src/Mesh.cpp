#include "Mesh.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

#include "VertexFormat.hpp"

void Mesh::CreateBuffers(void* vertexData, unsigned int vertexDataSize,
						 void* indexData, unsigned int indexDataSize)
{
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	// Create buffers
	glGenBuffers(2, bufferObjects);

	// Bind and upload index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufferObjects[IndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

	// Bind and upload vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, bufferObjects[VertexBuffer]);
	glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);
}

void Mesh::DeleteBuffers()
{
	glDeleteVertexArrays(1, &vertexArrayObject);
	glDeleteBuffers(2, bufferObjects);
}

void Mesh::Upload_3f2f(BufferRef<Vertex3f2f> vertices, BufferRef<unsigned short> indices)
{
	using V = Vertex3f2f;

	unsigned int verticesSize = static_cast<unsigned int>(V::size * vertices.count);
	unsigned int indicesSize = static_cast<unsigned int>(sizeof(unsigned short) * indices.count);

	this->indexCount = GLsizei(indices.count);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertices.data, verticesSize, indices.data, indicesSize);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_3f3f(BufferRef<Vertex3f3f> vertices, BufferRef<unsigned short> indices)
{
	using V = Vertex3f3f;

	unsigned int verticesSize = static_cast<unsigned int>(V::size * vertices.count);
	unsigned int indicesSize = static_cast<unsigned int>(sizeof(unsigned short) * indices.count);

	this->indexCount = GLsizei(indices.count);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertices.data, verticesSize, indices.data, indicesSize);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_3f3f2f(BufferRef<Vertex3f3f2f> vertices, BufferRef<unsigned short> indices)
{
	using V = Vertex3f3f2f;

	unsigned int verticesSize = static_cast<unsigned int>(V::size * vertices.count);
	unsigned int indicesSize = static_cast<unsigned int>(sizeof(unsigned short) * indices.count);

	this->indexCount = GLsizei(indices.count);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertices.data, verticesSize, indices.data, indicesSize);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::cElemCount, V::cElemType, GL_FALSE, V::size, V::cOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_3f3f3f(BufferRef<Vertex3f3f3f> vertices, BufferRef<unsigned short> indices)
{
	using V = Vertex3f3f3f;

	unsigned int verticesSize = static_cast<unsigned int>(V::size * vertices.count);
	unsigned int indicesSize = static_cast<unsigned int>(sizeof(unsigned short) * indices.count);

	this->indexCount = GLsizei(indices.count);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertices.data, verticesSize, indices.data, indicesSize);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::cElemCount, V::cElemType, GL_FALSE, V::size, V::cOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}
