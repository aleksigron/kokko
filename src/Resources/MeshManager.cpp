#include "Resources/MeshManager.hpp"

#include <cassert>

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/MeshLoader.hpp"

#include "System/File.hpp"

const MeshId MeshId::Null = MeshId{ 0 };

MeshManager::MeshManager(Allocator* allocator, RenderDevice* renderDevice) :
	allocator(allocator),
	renderDevice(renderDevice),
	pathHashMap(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as Null instance

	freeListFirst = 0;

	this->Reallocate(16);
}

MeshManager::~MeshManager()
{
	for (unsigned int i = 1; i < data.count; ++i)
	{
		allocator->Deallocate(data.pathString[i]);

		// DeleteBuffers will not double-delete,
		// so it's safe to call for every element
		DeleteBuffers(data.bufferData[i]);
	}

	allocator->Deallocate(data.buffer);
}

void MeshManager::Reallocate(unsigned int required)
{
	KOKKO_PROFILE_FUNCTION();

	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	size_t objectBytes = sizeof(unsigned int) + sizeof(MeshDrawData) +
		sizeof(MeshBufferData) + sizeof(BoundingBox) + sizeof(char*);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.drawData = reinterpret_cast<MeshDrawData*>(newData.freeList + required);
	newData.bufferData = reinterpret_cast<MeshBufferData*>(newData.drawData + required);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.bufferData + required);
	newData.pathString = reinterpret_cast<char**>(newData.bounds + required);

	if (data.buffer != nullptr)
	{
		// FIXME: This will not copy everything if you have holes in the buffer
		// This will happen if you remove meshes: count goes down, but the data spans the same area
		// This needs some real refactoring
		// I don't want to add levels of indirection because getting mesh information in
		// the render loop can be a bottleneck. I don't know that for sure, so I should probably
		// try it before making assumptions

		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memcpy(newData.drawData, data.drawData, data.count * sizeof(MeshDrawData));
		std::memcpy(newData.bufferData, data.bufferData, data.count * sizeof(MeshBufferData));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.pathString, data.pathString, data.count * sizeof(char*));

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

	data.bufferData[id.i] = MeshBufferData{};
	data.pathString[id.i] = nullptr;

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

	allocator->Deallocate(data.pathString[id.i]);
	data.pathString[id.i] = nullptr;

	DeleteBuffers(data.bufferData[id.i]);

	--data.count;
}

MeshId MeshManager::GetIdByPath(StringRef path)
{
	KOKKO_PROFILE_FUNCTION();

	uint32_t hash = Hash::FNV1a_32(path.str, path.len);

	HashMap<uint32_t, MeshId>::KeyValuePair* pair = pathHashMap.Lookup(hash);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Buffer<unsigned char> file(allocator);
	String pathStr(allocator, path);

	if (File::ReadBinary(pathStr.GetCStr(), file))
	{
		MeshId id = CreateMesh();
		MeshLoader loader(this);

		MeshLoader::Status status = loader.LoadFromBuffer(id, file.GetRef());
		if (status == MeshLoader::Status::Success)
		{
			pair = pathHashMap.Insert(hash);
			pair->second = id;

			// Copy path string
			data.pathString[id.i] = static_cast<char*>(allocator->Allocate(path.len + 1));
			std::memcpy(data.pathString[id.i], path.str, path.len);
			data.pathString[id.i][path.len] = '\0';

			return id;
		}
		else
		{
			RemoveMesh(id);
		}
	}

	return MeshId{};
}

MeshId MeshManager::GetIdByPathHash(uint32_t pathHash)
{
	auto pair = pathHashMap.Lookup(pathHash);
	return pair != nullptr ? pair->second : MeshId{};
}

const char* MeshManager::GetPath(MeshId id)
{
	return data.pathString[id.i];
}

void MeshManager::UpdateBuffers(MeshId id, const void* vertBuf, unsigned int vertBytes, RenderBufferUsage usage)
{
	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject == 0)
	{
		// Create vertex array object
		renderDevice->CreateVertexArrays(1, &bufferData.vertexArrayObject);
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Create buffer objects
		renderDevice->CreateBuffers(1, bufferData.bufferObjects);

		bufferData.bufferObjects[MeshBufferData::IndexBuffer] = 0;
		bufferData.bufferSizes[MeshBufferData::IndexBuffer] = 0;

		// Bind and upload vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);
		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vertBytes, vertBuf, usage);
		bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vertBytes;
	}
	else
	{
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Bind and update vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

		if (vertBytes <= bufferData.bufferSizes[MeshBufferData::VertexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer, 0, vertBytes, vertBuf);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vertBytes, vertBuf, usage);
			bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vertBytes;
		}
	}
}

void MeshManager::UpdateIndexedBuffers(
	MeshId id, const void* vertBuf, unsigned int vertBytes,
	const void* idxBuf, unsigned int idxBytes, RenderBufferUsage usage)
{
	MeshBufferData& bufferData = data.bufferData[id.i];

	if (bufferData.vertexArrayObject == 0)
	{
		// Create vertex array object
		renderDevice->CreateVertexArrays(1, &bufferData.vertexArrayObject);
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Create buffer objects
		renderDevice->CreateBuffers(2, bufferData.bufferObjects);

		// Bind and upload index buffer
		renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, bufferData.bufferObjects[MeshBufferData::IndexBuffer]);
		renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer, idxBytes, idxBuf, usage);
		bufferData.bufferSizes[MeshBufferData::IndexBuffer] = idxBytes;

		// Bind and upload vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);
		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vertBytes, vertBuf, usage);
		bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vertBytes;
	}
	else
	{
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Bind and update index buffer
		renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, bufferData.bufferObjects[MeshBufferData::IndexBuffer]);

		if (idxBytes <= bufferData.bufferSizes[MeshBufferData::IndexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::IndexBuffer, 0, idxBytes, idxBuf);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer, idxBytes, idxBuf, usage);
			bufferData.bufferSizes[MeshBufferData::IndexBuffer] = idxBytes;
		}

		// Bind and update vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

		if (vertBytes <= bufferData.bufferSizes[MeshBufferData::VertexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer, 0, vertBytes, vertBuf);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer, vertBytes, vertBuf, usage);
			bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vertBytes;
		}
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

void MeshManager::CreateDrawData(MeshId id, const VertexData& vdata)
{
	MeshDrawData& drawData = data.drawData[id.i];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[id.i].vertexArrayObject;
	drawData.count = vdata.vertexCount;
	drawData.indexType = RenderIndexType::None;
}

void MeshManager::CreateDrawDataIndexed(MeshId id, const IndexedVertexData& vdata)
{
	MeshDrawData& drawData = data.drawData[id.i];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[id.i].vertexArrayObject;
	drawData.count = vdata.indexCount;
	drawData.indexType = RenderIndexType::UnsignedShort;
}

void MeshManager::SetVertexAttribPointers(const VertexFormat& vertexFormat)
{
	for (unsigned int i = 0; i < vertexFormat.attributeCount; ++i)
	{
		const VertexAttribute& attr = vertexFormat.attributes[i];

		renderDevice->EnableVertexAttribute(attr.attrIndex);

		RenderCommandData::SetVertexAttributePointer data{
			attr.attrIndex, attr.elemCount, attr.elemType, vertexFormat.vertexSize, attr.offset
		};

		renderDevice->SetVertexAttributePointer(&data);
	}
}

void MeshManager::Upload(MeshId id, const VertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.vertexFormat.attributes != nullptr && vdata.vertexFormat.attributeCount > 0);

	unsigned int vsize = vdata.vertexFormat.vertexSize * vdata.vertexCount;

	UpdateBuffers(id, vdata.vertexData, vsize, vdata.usage);
	CreateDrawData(id, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}

void MeshManager::UploadIndexed(MeshId id, const IndexedVertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.vertexFormat.attributes != nullptr && vdata.vertexFormat.attributeCount > 0);

	unsigned int vsize = vdata.vertexFormat.vertexSize * vdata.vertexCount;
	unsigned int isize = vdata.indexSize * vdata.indexCount;

	UpdateIndexedBuffers(id, vdata.vertexData, vsize, vdata.indexData, isize, vdata.usage);
	CreateDrawDataIndexed(id, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}

