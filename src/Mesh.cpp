#include "Mesh.hpp"

#include <cassert>

#include "IncludeOpenGL.hpp"

#include "VertexFormat.hpp"

const unsigned int Mesh::primitiveModeValues[] = {
	GL_POINTS,
	GL_LINE_STRIP,
	GL_LINE_LOOP,
	GL_LINES,
	GL_TRIANGLE_STRIP,
	GL_TRIANGLE_FAN,
	GL_TRIANGLES
};

Mesh::Mesh() :
	vertexArrayObject(0),
	bufferObjects{ 0, 0 },
	indexCount(0),
	indexElementType(0),
	primitiveMode(GL_TRIANGLES)
{
}

Mesh& Mesh::operator=(Mesh&& other)
{
	// Delete possible existing buffers
	this->DeleteBuffers();

	// Set values from other Mesh instance
	this->vertexArrayObject = other.vertexArrayObject;
	this->bufferObjects[0] = other.bufferObjects[0];
	this->bufferObjects[1] = other.bufferObjects[1];
	this->indexCount = other.indexCount;
	this->indexElementType = other.indexElementType;
	this->primitiveMode = other.primitiveMode;
	this->bounds = other.bounds;

	// Clear values in other Mesh instance
	other.vertexArrayObject = 0;
	other.bufferObjects[0] = 0;
	other.bufferObjects[1] = 0;
	other.indexCount = 0;
	other.indexElementType = 0;

	return *this;
}

Mesh::~Mesh()
{
	this->DeleteBuffers();
}

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
	if (vertexArrayObject != 0)
	{
		glDeleteVertexArrays(1, &vertexArrayObject);
		glDeleteBuffers(2, bufferObjects);
	}
}

void Mesh::Upload_3f(BufferRef<Vertex3f> vertices, BufferRef<unsigned short> indices)
{
	using V = Vertex3f;

	unsigned int verticesSize = static_cast<unsigned int>(V::size * vertices.count);
	unsigned int indicesSize = static_cast<unsigned int>(sizeof(unsigned short) * indices.count);

	this->indexCount = GLsizei(indices.count);
	this->indexElementType = GL_UNSIGNED_SHORT;

	this->CreateBuffers(vertices.data, verticesSize, indices.data, indicesSize);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	// Unbind vertex array
	glBindVertexArray(0);
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


bool Mesh::LoadFromBuffer(BufferRef<unsigned char> buffer)
{
	using uint = unsigned int;
	using ushort = unsigned short;
	using ubyte = unsigned char;

	const uint headerSize = 16;
	const uint boundsSize = 6 * sizeof(float);

	if (buffer.IsValid() && buffer.count >= headerSize)
	{
		ubyte* d = buffer.data;
		uint* headerData = reinterpret_cast<uint*>(d);
		uint fileMagic = headerData[0];

		if (fileMagic == 0x91191010)
		{
			// Get header data

			uint vertComps = headerData[1];
			uint vertCount = headerData[2];
			uint indexCount = headerData[3];

			// Get vertex data components count and size

			uint posCount = (vertComps & 0x01) >> 0;
			uint posSize = posCount * 3 * sizeof(float);

			uint normCount = (vertComps & 0x02) >> 1;
			uint normSize = normCount * 3 * sizeof(float);

			uint colCount = (vertComps & 0x04) >> 2;
			uint colSize = colCount * 3 * sizeof(float);

			uint texCount = (vertComps & 0x08) >> 3;
			uint texSize = texCount * 2 * sizeof(float);

			uint vertSize = posSize + normSize + colSize + texSize;
			uint vertexDataSize = vertCount * vertSize;

			uint indexSize = indexCount > (1 << 16) ? 4 : 2;
			uint indexDataSize = indexCount * indexSize;

			uint expectedSize = headerSize + boundsSize + vertexDataSize + indexDataSize;

			assert(expectedSize == buffer.count);

			// Check that the file size matches the header description
			if (expectedSize == buffer.count)
			{
				float* boundsData = reinterpret_cast<float*>(d + headerSize);
				ubyte* vertData = d + headerSize + boundsSize;
				ushort* indexData = reinterpret_cast<ushort*>(d + headerSize + boundsSize + vertexDataSize);

				this->bounds.center.x = boundsData[0];
				this->bounds.center.y = boundsData[1];
				this->bounds.center.z = boundsData[2];

				this->bounds.extents.x = boundsData[3];
				this->bounds.extents.y = boundsData[4];
				this->bounds.extents.z = boundsData[5];

				this->SetPrimitiveMode(Mesh::PrimitiveMode::Triangles);

				if (normCount == 0 && colCount == 0 && texCount == 1)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f2f> vertices;
					vertices.data = reinterpret_cast<Vertex3f2f*>(vertData);
					vertices.count = vertCount;

					this->Upload_3f2f(vertices, indices);
				}
				else if ((normCount == 1 && colCount == 0 && texCount == 0) ||
						 (normCount == 0 && colCount == 1 && texCount == 0))
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f*>(vertData);
					vertices.count = vertCount;

					this->Upload_3f3f(vertices, indices);
				}
				else if (normCount == 1 && colCount == 0 && texCount == 1)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f2f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f2f*>(vertData);
					vertices.count = vertCount;

					this->Upload_3f3f2f(vertices, indices);
				}
				else if (normCount == 1 && colCount == 1 && texCount == 0)
				{
					BufferRef<unsigned short> indices(indexData, indexCount);
					BufferRef<Vertex3f3f3f> vertices;
					vertices.data = reinterpret_cast<Vertex3f3f3f*>(vertData);
					vertices.count = vertCount;

					this->Upload_3f3f3f(vertices, indices);
				}
				
				return true;
			}
		}
	}
	
	return false;
}
