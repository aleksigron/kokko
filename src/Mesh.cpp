#include "Mesh.hpp"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

void Mesh::CreateBuffers(void* vertexData, unsigned int vertexDataSize,
						 void* indexData, unsigned int indexDataSize)
{
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	// Bind and upload index buffer
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSize, indexData, GL_STATIC_DRAW);

	// Bind and upload vertex buffer
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexDataSize, vertexData, GL_STATIC_DRAW);
}

void Mesh::Upload_PosNor(float* vertexData, unsigned int vertexCount,
						 unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosNor;

	this->indexCount = GLsizei(indexCount);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertexData, V::size * vertexCount,
						indexData, sizeof(uint16_t) * indexCount);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::norElements, V::norElemType,
						  GL_FALSE, V::size, V::norOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_PosCol(float* vertexData, unsigned int vertexCount,
						 unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosCol;

	this->indexCount = GLsizei(indexCount);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertexData, V::size * vertexCount,
						indexData, sizeof(uint16_t) * indexCount);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::colElements, V::colElemType,
						  GL_FALSE, V::size, V::colOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_PosNorCol(float* vertexData, unsigned int vertexCount,
							unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosNorCol;

	this->indexCount = GLsizei(indexCount);
	this->indexElementType = GL_UNSIGNED_SHORT;
	
	this->CreateBuffers(vertexData, V::size * vertexCount,
						indexData, sizeof(uint16_t) * indexCount);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::norElements, V::norElemType,
						  GL_FALSE, V::size, V::norOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::colElements, V::colElemType,
						  GL_FALSE, V::size, V::colOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_PosTex(float* vertexData, unsigned int vertexCount,
						 unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosTex;

	this->indexCount = GLsizei(indexCount);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertexData, V::size * vertexCount,
						indexData, sizeof(uint16_t) * indexCount);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::texCoordElements, V::texCoordElemType,
						  GL_FALSE, V::size, V::texCoordOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::Upload_PosNorTex(float* vertexData, unsigned int vertexCount,
							unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosNorTex;

	this->indexCount = GLsizei(indexCount);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertexData, V::size * vertexCount,
						indexData, sizeof(uint16_t) * indexCount);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::norElements, V::norElemType,
						  GL_FALSE, V::size, V::norOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::texCoordElements, V::texCoordElemType,
						  GL_FALSE, V::size, V::texCoordOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}
