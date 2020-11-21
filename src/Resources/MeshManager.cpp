#include "Resources/MeshManager.hpp"

#include <cassert>

#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/MeshLoader.hpp"

#include "System/File.hpp"
#include "System/IncludeOpenGL.hpp"

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

MeshManager::MeshManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	nameHashMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(8);
}

MeshManager::~MeshManager()
{
	for (unsigned int i = 1; i < data.count; ++i)
	{
		// DeleteBuffers will not double-delete,
		// so it's safe to call for every element
		DeleteBuffers(data.bufferData[i]);
	}

	allocator->Deallocate(data.buffer);
}

void MeshManager::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	unsigned int objectBytes = sizeof(unsigned int) + sizeof(MeshDrawData) +
		sizeof(MeshBufferData) + sizeof(BoundingBox);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes);
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

		allocator->Deallocate(data.buffer);
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
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<unsigned char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadBinary(pathStr.GetCStr(), file))
	{
		MeshId id = CreateMesh();
		MeshLoader loader(this, id);

		if (loader.LoadFromBuffer(file.GetRef()))
		{
			pair = nameHashMap.Insert(hash);
			pair->second = id;

			return id;
		}
		else
		{
			RemoveMesh(id);
		}
	}

	return MeshId{};
}

MeshBufferData MeshManager::CreateBuffers(
	const void* vd, unsigned int vs,
	RenderBufferUsage usage)
{
	MeshBufferData data;

	// Create vertex array object
	renderDevice->CreateVertexArrays(1, &data.vertexArrayObject);
	renderDevice->BindVertexArray(data.vertexArrayObject);

	// Create buffer objects
	renderDevice->CreateBuffers(1, data.bufferObjects);
	data.bufferObjects[MeshBufferData::IndexBuffer] = 0;

	// Bind and upload vertex buffer
	renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, data.bufferObjects[MeshBufferData::VertexBuffer]);
	renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vs, vd, usage);
	data.bufferSizes[MeshBufferData::VertexBuffer] = vs;

	return data;
}

MeshBufferData MeshManager::CreateIndexedBuffers(
	const void* vd, unsigned int vs, const void* id, unsigned int is,
	RenderBufferUsage usage)
{
	MeshBufferData data;

	// Create vertex array object
	renderDevice->CreateVertexArrays(1, &data.vertexArrayObject);
	renderDevice->BindVertexArray(data.vertexArrayObject);

	// Create buffer objects
	renderDevice->CreateBuffers(2, data.bufferObjects);

	// Bind and upload index buffer
	renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, data.bufferObjects[MeshBufferData::IndexBuffer]);
	renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer, is, id, usage);
	data.bufferSizes[MeshBufferData::IndexBuffer] = is;

	// Bind and upload vertex buffer
	renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, data.bufferObjects[MeshBufferData::VertexBuffer]);
	renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vs, vd, usage);
	data.bufferSizes[MeshBufferData::VertexBuffer] = vs;

	return data;
}

void MeshManager::UpdateBuffers(MeshBufferData& bufferDataInOut,
	const void* vd, unsigned int vs,
	RenderBufferUsage usage)
{
	assert(bufferDataInOut.vertexArrayObject != 0);

	renderDevice->BindVertexArray(bufferDataInOut.vertexArrayObject);

	// Bind and update vertex buffer
	renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferDataInOut.bufferObjects[MeshBufferData::VertexBuffer]);

	if (vs <= bufferDataInOut.bufferSizes[MeshBufferData::VertexBuffer])
	{
		// Only update the part of the buffer we need
		renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer, 0, vs, vd);
	}
	else
	{
		// SetBufferData reallocates storage when needed
		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vs, vd, usage);
		bufferDataInOut.bufferSizes[MeshBufferData::VertexBuffer] = vs;
	}
}

void MeshManager::UpdateIndexedBuffers(MeshBufferData& bufferDataInOut,
	const void* vd, unsigned int vs, const void* id, unsigned int is,
	RenderBufferUsage usage)
{
	assert(bufferDataInOut.vertexArrayObject != 0);

	renderDevice->BindVertexArray(bufferDataInOut.vertexArrayObject);

	// Bind and update index buffer
	renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, bufferDataInOut.bufferObjects[MeshBufferData::IndexBuffer]);

	if (vs <= bufferDataInOut.bufferSizes[MeshBufferData::IndexBuffer])
	{
		// Only update the part of the buffer we need
		renderDevice->SetBufferSubData(RenderBufferTarget::IndexBuffer, 0, is, id);
	}
	else
	{
		// SetBufferData reallocates storage when needed
		renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer, is, id, usage);
		bufferDataInOut.bufferSizes[MeshBufferData::IndexBuffer] = is;
	}

	// Bind and update vertex buffer
	renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferDataInOut.bufferObjects[MeshBufferData::VertexBuffer]);

	if (vs <= bufferDataInOut.bufferSizes[MeshBufferData::VertexBuffer])
	{
		// Only update the part of the buffer we need
		renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer, 0, vs, vd);
	}
	else
	{
		// SetBufferData reallocates storage when needed
		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vs, vd, usage);
		bufferDataInOut.bufferSizes[MeshBufferData::VertexBuffer] = vs;
	}
}

void MeshManager::DeleteBuffers(MeshBufferData& bufferDataInOut) const
{
	if (bufferDataInOut.vertexArrayObject != 0)
	{
		renderDevice->DestroyVertexArrays(1, &bufferDataInOut.vertexArrayObject);
		renderDevice->DestroyBuffers(2, bufferDataInOut.bufferObjects);

		bufferDataInOut.vertexArrayObject = 0;
		bufferDataInOut.bufferObjects[0] = 0;
		bufferDataInOut.bufferObjects[1] = 0;
		bufferDataInOut.bufferSizes[0] = 0;
		bufferDataInOut.bufferSizes[1] = 0;
	}
}

void MeshManager::Upload_3f(MeshId id, VertexData<Vertex3f> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateBuffers(bufferData, vdata.vertData, vertSize, usage);
	}
	else
	{
		bufferData = CreateBuffers(vdata.vertData, vertSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.vertCount;
	drawData.indexType = 0;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);

	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}


void MeshManager::UploadIndexed_3f(MeshId id, IndexedVertexData<Vertex3f, unsigned short> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateIndexedBuffers(bufferData, vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}
	else
	{
		bufferData = CreateIndexedBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.idxCount;
	drawData.indexType = GL_UNSIGNED_SHORT;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);

	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}

void MeshManager::Upload_3f2f(MeshId id, IndexedVertexData<Vertex3f2f, unsigned short> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f2f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateIndexedBuffers(bufferData, vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}
	else
	{
		bufferData = CreateIndexedBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.idxCount;
	drawData.indexType = GL_UNSIGNED_SHORT;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);
	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	renderDevice->EnableVertexAttribute(1);
	RenderCommandData::SetVertexAttributePointer b{
		1, V::bElemCount, V::bElemType, V::size, V::bOffset
	};
	renderDevice->SetVertexAttributePointer(&b);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}

void MeshManager::Upload_3f3f(MeshId id, IndexedVertexData<Vertex3f3f, unsigned short> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateIndexedBuffers(bufferData, vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}
	else
	{
		bufferData = CreateIndexedBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.idxCount;
	drawData.indexType = GL_UNSIGNED_SHORT;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);
	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	renderDevice->EnableVertexAttribute(1);
	RenderCommandData::SetVertexAttributePointer b{
		1, V::bElemCount, V::bElemType, V::size, V::bOffset
	};
	renderDevice->SetVertexAttributePointer(&b);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}

void MeshManager::Upload_3f3f2f(MeshId id, IndexedVertexData<Vertex3f3f2f, unsigned short> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f3f2f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateIndexedBuffers(bufferData, vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}
	else
	{
		bufferData = CreateIndexedBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.idxCount;
	drawData.indexType = GL_UNSIGNED_SHORT;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);
	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	renderDevice->EnableVertexAttribute(1);
	RenderCommandData::SetVertexAttributePointer b{
		1, V::bElemCount, V::bElemType, V::size, V::bOffset
	};
	renderDevice->SetVertexAttributePointer(&b);

	renderDevice->EnableVertexAttribute(2);
	RenderCommandData::SetVertexAttributePointer c{
		2, V::cElemCount, V::cElemType, V::size, V::cOffset
	};
	renderDevice->SetVertexAttributePointer(&c);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}

void MeshManager::Upload_3f3f3f(MeshId id, IndexedVertexData<Vertex3f3f3f, unsigned short> vdata, RenderBufferUsage usage)
{
	using V = Vertex3f3f3f;

	unsigned int vertSize = static_cast<unsigned int>(V::size * vdata.vertCount);
	unsigned int idxSize = static_cast<unsigned int>(sizeof(unsigned short) * vdata.idxCount);

	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject != 0)
	{
		UpdateIndexedBuffers(bufferData, vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}
	else
	{
		bufferData = CreateIndexedBuffers(vdata.vertData, vertSize, vdata.idxData, idxSize, usage);
	}

	MeshDrawData drawData;
	drawData.primitiveMode = PrimitiveModeValue(vdata.primitiveMode);
	drawData.vertexArrayObject = bufferData.vertexArrayObject;
	drawData.count = vdata.idxCount;
	drawData.indexType = GL_UNSIGNED_SHORT;

	data.drawData[id.i] = drawData;

	renderDevice->EnableVertexAttribute(0);
	RenderCommandData::SetVertexAttributePointer a{
		0, V::aElemCount, V::aElemType, V::size, V::aOffset
	};
	renderDevice->SetVertexAttributePointer(&a);

	renderDevice->EnableVertexAttribute(1);
	RenderCommandData::SetVertexAttributePointer b{
		1, V::bElemCount, V::bElemType, V::size, V::bOffset
	};
	renderDevice->SetVertexAttributePointer(&b);

	renderDevice->EnableVertexAttribute(2);
	RenderCommandData::SetVertexAttributePointer c{
		2, V::cElemCount, V::cElemType, V::size, V::cOffset
	};
	renderDevice->SetVertexAttributePointer(&c);

	// Unbind vertex array
	renderDevice->BindVertexArray(0);
}
