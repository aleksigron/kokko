#include "RenderObject.h"

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

RenderObject::RenderObject()
{
}

RenderObject::~RenderObject()
{
	if (this->vertexBufferObject != 0)
		glDeleteBuffers(1, &this->vertexBufferObject);
	
	if (this->vertexArrayObject != 0)
		glDeleteVertexArrays(1, &this->vertexArrayObject);
}

void RenderObject::SetVertexBufferData(Buffer<Vec3f>& vertexBuffer)
{
	glGenVertexArrays(1, &this->vertexArrayObject);
	glBindVertexArray(this->vertexArrayObject);
	
	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &this->vertexBufferObject);
	
	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, this->vertexBufferObject);
	
	// Give our vertices to OpenGL.
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vec3f) * vertexBuffer.Count(), vertexBuffer.Data(), GL_STATIC_DRAW);
	
	this->shader.LoadShaders("res/shaders/simple.vert", "res/shaders/simple.frag");
}