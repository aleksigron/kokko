#include "RenderObject.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

void RenderObject::UploadVertexData_PosCol(const Buffer<uint16_t>& index, const Buffer<Vertex_PosCol>& vertex)
{
	using Vert = Vertex_PosCol;
	
	// Create vertex array object
	glGenVertexArrays(1, &vertexArrayObject);
	glBindVertexArray(vertexArrayObject);
	
	// Bind and upload index buffer
	indexCount = GLsizei(index.Count());
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * index.Count(), index.Data(), GL_STATIC_DRAW);
	
	// Bind and upload vertex buffer
	vertexCount = GLsizei(vertex.Count());
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, Vert::size * vertex.Count(), vertex.Data(), GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, Vert::posElements, Vert::posElemType, GL_FALSE, Vert::size, Vert::posOffset);
	
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, Vert::colElements, Vert::colElemType, GL_FALSE, Vert::size, Vert::colOffset);
	
	// Unbind vertex array
	glBindVertexArray(0);
}