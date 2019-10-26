#include "MeshManager.hpp"

#include <cassert>

#include "IncludeOpenGL.hpp"

#include "Hash.hpp"
#include "File.hpp"
#include "MeshLoader.hpp"

static unsigned int PrimitiveModeValue(MeshPrimitiveMode mode)
{
	switch (mode) {
		case MeshPrimitiveMode::Points: return GL_POINTS;
		case MeshPrimitiveMode::LineStrip: return GL_LINE_STRIP;
		case MeshPrimitiveMode::LineLoop: return GL_LINE_LOOP;
		case MeshPrimitiveMode::Lines: return GL_LINES;
		case MeshPrimitiveMode::TriangleStrip: return GL_TRIANGLE_STRIP;
		case MeshPrimitiveMode::TriangleFan: return GL_TRIANGLE_FAN;
		case MeshPrimitiveMode::Triangles: return GL_TRIANGLES;
		default: return 0;
	}
}

MeshManager::MeshManager()
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(8);
}

MeshManager::~MeshManager()
{
	// TODO: Release resources

	operator delete[](data.buffer);
}

void MeshManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	unsigned int objectBytes = sizeof(unsigned int) + sizeof(MeshDrawData) +
		sizeof(MeshBufferData) + sizeof(BoundingBox);

	InstanceData newData;
	newData.buffer = operator new[](required * objectBytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.drawData = reinterpret_cast<MeshDrawData*>(newData.freeList + required);
	newData.bufferData = reinterpret_cast<MeshBufferData*>(newData.drawData + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.bufferData + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memcpy(newData.drawData, data.drawData, data.count * sizeof(MeshDrawData));
		std::memcpy(newData.bufferData, data.bufferData, data.count * sizeof(MeshBufferData));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));

		operator delete[](data.buffer);
	}

	data = newData;
}

MeshId MeshManager::CreateMesh()
{
	MeshId id;

	if (freeListFirst == 0)
	{
		if (data.count == data.allocated)
			this->Reallocate(data.count + 1);

		// If there are no freelist entries, first <objectCount> indices must be in use
		id.i = data.count;
	}
	else
	{
		id.i = freeListFirst;
		freeListFirst = data.freeList[freeListFirst];
	}

	// Clear buffer data
	data.bufferData[id.i] = MeshBufferData{};

	++data.count;

	return id;
}

void DeleteBuffers(MeshBufferData& buffers)
{
	if (buffers.vertexArrayObject != 0)
	{
		glDeleteVertexArrays(1, &buffers.vertexArrayObject);
		glDeleteBuffers(2, buffers.bufferObjects);

		buffers.vertexArrayObject = 0;
		buffers.bufferObjects[0] = 0;
		buffers.bufferObjects[1] = 0;
	}
}

void MeshManager::RemoveMesh(MeshId id)
{
	// Mesh isn't the last one
	if (id.i < data.count - 1)
	{
		data.freeList[id.i] = freeListFirst;
		freeListFirst = id.i;
	}

	DeleteBuffers(data.bufferData[id.i]);

	--data.count;
}

MeshId MeshManager::GetIdByPath(StringRef path)
{
	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, MeshId>::KeyValuePair* pair = nameHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->value;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<unsigned char> file = File::ReadBinary(path);

	if (file.IsValid())
	{
		MeshId id = CreateMesh();
		MeshLoader loader(this, id);

		if (loader.LoadFromBuffer(file.GetRef()))
		{
			pair = nameHashMap.Insert(hash);
			pair->value = id;

			return id;
		}
		else
		{
			RemoveMesh(id);
		}
	}

	return MeshId{};
}

MeshBufferData CreateBuffers(const void* vd, unsigned int vs, const void* id, unsigned int is)
{
	MeshBufferData data;

	// Create vertex array object
	glGenVertexArrays(1, &data.vertexArrayObject);
	glBindVertexArray(data.vertexArrayObject);

	// Create buffer objects
	glGenBuffers(2, data.bufferObjects);

	// Bind and upload index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.bufferObjects[MeshBufferData::IndexBuffer]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, is, id, GL_STATIC_DRAW);

	// Bind and upload vertex buffer
	glBindBuffer(GL_ARRAY_BUFFER, data.bufferObjects[MeshBufferData::VertexBuffer]);
	glBufferData(GL_ARRAY_BUFFER, vs, vd, GL_STATIC_DRAW);

	return data;
}

void MeshManager::Upload_3f(MeshId id, IndexedVertexData<Vertex3f, unsigned short> vdata)
{
	DeleteBuffers(data.bufferData[id.i]);

	using V = Vertex3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData bufferData = CreateBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize);

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.indexCount = vdata.idxCount;
	drawData.indexElementType = GL_UNSIGNED_SHORT;

	data.bufferData[id.i] = bufferData;
	data.drawData[id.i] = drawData;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void MeshManager::Upload_3f2f(MeshId id, IndexedVertexData<Vertex3f2f, unsigned short> vdata)
{
	DeleteBuffers(data.bufferData[id.i]);

	using V = Vertex3f2f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData bufferData = CreateBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize);

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.indexCount = vdata.idxCount;
	drawData.indexElementType = GL_UNSIGNED_SHORT;

	data.bufferData[id.i] = bufferData;
	data.drawData[id.i] = drawData;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void MeshManager::Upload_3f3f(MeshId id, IndexedVertexData<Vertex3f3f, unsigned short> vdata)
{
	DeleteBuffers(data.bufferData[id.i]);

	using V = Vertex3f3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData bufferData = CreateBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize);

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.indexCount = vdata.idxCount;
	drawData.indexElementType = GL_UNSIGNED_SHORT;

	data.bufferData[id.i] = bufferData;
	data.drawData[id.i] = drawData;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void MeshManager::Upload_3f3f2f(MeshId id, IndexedVertexData<Vertex3f3f2f, unsigned short> vdata)
{
	DeleteBuffers(data.bufferData[id.i]);

	using V = Vertex3f3f2f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData bufferData = CreateBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize);

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.indexCount = vdata.idxCount;
	drawData.indexElementType = GL_UNSIGNED_SHORT;

	data.bufferData[id.i] = bufferData;
	data.drawData[id.i] = drawData;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::cElemCount, V::cElemType, GL_FALSE, V::size, V::cOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}

void MeshManager::Upload_3f3f3f(MeshId id, IndexedVertexData<Vertex3f3f3f, unsigned short> vdata)
{
	DeleteBuffers(data.bufferData[id.i]);

	using V = Vertex3f3f3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData bufferData = CreateBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize);

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.indexCount = vdata.idxCount;
	drawData.indexElementType = GL_UNSIGNED_SHORT;

	data.bufferData[id.i] = bufferData;
	data.drawData[id.i] = drawData;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, V::aElemCount, V::aElemType, GL_FALSE, V::size, V::aOffset);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, V::bElemCount, V::bElemType, GL_FALSE, V::size, V::bOffset);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, V::cElemCount, V::cElemType, GL_FALSE, V::size, V::cOffset);

	// Unbind vertex array
	glBindVertexArray(0);
}
