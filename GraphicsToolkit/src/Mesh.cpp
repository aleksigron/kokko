#include "Mesh.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

void Mesh::Upload_PosCol(float* vertexData, unsigned int vertexCount,
						 unsigned short* indexData, unsigned int indexCount)
{
	using V = Vertex_PosCol;

	// Create vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	this->indexCount = GLsizei(indexCount);
	indexElementType = GL_UNSIGNED_SHORT;

	int error0 = glGetError();

	// Bind and upload index buffer
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(uint16_t) * indexCount, indexData, GL_STATIC_DRAW);


	int error1 = glGetError();

	// Bind and upload vertex buffer
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,
				 V::size * vertexCount, vertexData, GL_STATIC_DRAW);

	int error2 = glGetError();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	int error3 = glGetError();

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::colElements, V::colElemType,
						  GL_FALSE, V::size, V::colOffset);

	int error4 = glGetError();

	// Unbind vertex array
	glBindVertexArray(0);
}

void Mesh::UploadVertexData_PosTex(const Buffer<unsigned short>& index,
								   const Buffer<Vertex_PosTex>& vertex)
{
	using V = Vertex_PosTex;

	// Create vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);

	indexCount = GLsizei(index.Count());
	indexElementType = GL_UNSIGNED_SHORT;

	// Bind and upload index buffer
	GLuint indexBuffer;
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
				 sizeof(uint16_t) * index.Count(), index.Data(), GL_STATIC_DRAW);

	// Bind and upload vertex buffer
	GLuint vertexBuffer;
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER,
				 V::size * vertex.Count(), vertex.Data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::posElements, V::posElemType,
						  GL_FALSE, V::size, V::posOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::texCoordElements, V::texCoordElemType,
						  GL_FALSE, V::size, V::texCoordOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}