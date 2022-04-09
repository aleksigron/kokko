#include "Resources/MeshManager.hpp"

#include <cassert>

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/AssetLoader.hpp"

const MeshId MeshId::Null = MeshId{ 0 };

MeshManager::MeshManager(Allocator* allocator, kokko::AssetLoader* assetLoader, RenderDevice* renderDevice) :
	allocator(allocator),
	assetLoader(assetLoader),
	renderDevice(renderDevice),
	uidMap(allocator)
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

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	size_t objectBytes = sizeof(unsigned int) * 2 + sizeof(MeshDrawData) +
		sizeof(MeshBufferData) + sizeof(BoundingBox) + sizeof(MeshId) + sizeof(kokko::Uid);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.freeList = static_cast<unsigned int*>(newData.buffer);
	newData.indexList = newData.freeList + newData.allocated;
	newData.drawData = reinterpret_cast<MeshDrawData*>(newData.indexList + newData.allocated);
	newData.bufferData = reinterpret_cast<MeshBufferData*>(newData.drawData + newData.allocated);
	newData.uniqueVertexCount = reinterpret_cast<int*>(newData.bufferData + newData.allocated);
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.bufferData + newData.allocated);
	newData.meshId = reinterpret_cast<MeshId*>(newData.bounds + newData.allocated);
	newData.uid = reinterpret_cast<kokko::Uid*>(newData.meshId + newData.allocated);
	newData.uidExists = reinterpret_cast<bool*>(newData.uid + newData.allocated);

	if (data.buffer != nullptr)
	{
		constexpr size_t uintSize = sizeof(unsigned int);

		std::memcpy(newData.freeList, data.freeList, data.allocated * uintSize);
		std::memset(newData.freeList + data.allocated, 0, (newData.allocated - data.allocated) * uintSize);

		std::memcpy(newData.indexList, data.indexList, data.allocated * uintSize);
		std::memset(newData.indexList + data.allocated, 0, (newData.allocated - data.allocated) * uintSize);

		std::memcpy(newData.drawData, data.drawData, data.count * sizeof(MeshDrawData));
		std::memcpy(newData.bufferData, data.bufferData, data.count * sizeof(MeshBufferData));
		std::memcpy(newData.uniqueVertexCount, data.uniqueVertexCount, data.count * sizeof(int));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.meshId, data.meshId, data.count * sizeof(MeshId));
		std::memcpy(newData.uid, data.uid, data.count * sizeof(kokko::Uid));
		std::memcpy(newData.uidExists, data.uidExists, data.count * sizeof(bool));

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

	unsigned int index = data.count;
	data.indexList[id.i] = index;

	data.drawData[index] = MeshDrawData{};
	data.bufferData[index] = MeshBufferData{};
	data.uniqueVertexCount[index] = 0;
	data.bounds[index] = BoundingBox();
	data.meshId[index] = id;
	data.uid[index] = kokko::Uid();
	data.uidExists[index] = false;

	++data.count;

	return id;
}

void MeshManager::RemoveMesh(MeshId id)
{
	// Add removed MeshId to the free list
	data.freeList[id.i] = freeListFirst;
	freeListFirst = id.i;

	unsigned int index = GetIndex(id);

	DeleteBuffers(data.bufferData[index]);

	// Mesh isn't the last one in the data array
	if (index + 1 < data.count)
	{
		unsigned int lastIndex = data.count - 1;

		MeshId lastMeshId = data.meshId[lastIndex];

		// Move the last mesh into the removed mesh's place
		data.drawData[index] = data.drawData[lastIndex];
		data.bufferData[index] = data.bufferData[lastIndex];
		data.uniqueVertexCount[index] = data.uniqueVertexCount[lastIndex];
		data.bounds[index] = data.bounds[lastIndex];
		data.meshId[index] = lastMeshId;
		data.uid[index] = data.uid[lastIndex];
		data.uidExists[index] = data.uidExists[lastIndex];

		// Update index list
		data.indexList[lastMeshId.i] = index;
	}

	--data.count;
}

unsigned int MeshManager::GetIndex(MeshId meshId) const
{
	unsigned int index = data.indexList[meshId.i];
	assert(index != 0);
	return index;
}

Optional<kokko::Uid> MeshManager::GetUid(MeshId id) const
{
	unsigned int index = GetIndex(id);
	if (data.uidExists[index])
		return data.uid[index];
	else
		return Optional<kokko::Uid>();
}

void MeshManager::SetUid(MeshId id, const kokko::Uid& uid)
{
	unsigned int index = GetIndex(id);
	data.uid[index] = uid;
	data.uidExists[index] = true;
}

const BoundingBox* MeshManager::GetBoundingBox(MeshId id) const
{
	return &data.bounds[GetIndex(id)];
}

void MeshManager::SetBoundingBox(MeshId id, const BoundingBox& bounds)
{
	data.bounds[GetIndex(id)] = bounds;
}

const MeshDrawData* MeshManager::GetDrawData(MeshId id) const
{
	return &data.drawData[GetIndex(id)];
}

int MeshManager::GetUniqueVertexCount(MeshId id) const
{
	return data.uniqueVertexCount[GetIndex(id)];
}

void MeshManager::UpdateBuffers(unsigned int index, const VertexData& vdata)
{
	MeshBufferData& bufferData = data.bufferData[index];

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
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer,
			bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer,
			vdata.vertexDataSize, vdata.vertexData, vdata.vertexBufferUsage);

		bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
	}
	else
	{
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Bind and update vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

		if (vdata.vertexDataSize <= bufferData.bufferSizes[MeshBufferData::VertexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer,
				0, vdata.vertexDataSize, vdata.vertexData);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer,
				vdata.vertexDataSize, vdata.vertexData, vdata.vertexBufferUsage);
			bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
		}
	}
}

void MeshManager::UpdateBuffersIndexed(unsigned int index, const IndexedVertexData& vdata)
{
	MeshBufferData& bufferData = data.bufferData[index];

	if (bufferData.vertexArrayObject == 0)
	{
		// Create vertex array object
		renderDevice->CreateVertexArrays(1, &bufferData.vertexArrayObject);
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Create buffer objects
		renderDevice->CreateBuffers(2, bufferData.bufferObjects);

		// Bind and upload index buffer
		renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer,
			bufferData.bufferObjects[MeshBufferData::IndexBuffer]);
		renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer,
			vdata.indexDataSize, vdata.indexData, vdata.indexBufferUsage);
		bufferData.bufferSizes[MeshBufferData::IndexBuffer] = vdata.indexDataSize;

		// Bind and upload vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer,
			bufferData.bufferObjects[MeshBufferData::VertexBuffer]);
		renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer,
			vdata.vertexDataSize, vdata.vertexData, vdata.vertexBufferUsage);
		bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
	}
	else
	{
		renderDevice->BindVertexArray(bufferData.vertexArrayObject);

		// Bind and update index buffer
		renderDevice->BindBuffer(RenderBufferTarget::IndexBuffer, bufferData.bufferObjects[MeshBufferData::IndexBuffer]);

		if (vdata.vertexDataSize <= bufferData.bufferSizes[MeshBufferData::IndexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::IndexBuffer,
				0, vdata.indexDataSize, vdata.indexData);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::IndexBuffer,
				vdata.indexDataSize, vdata.indexData, vdata.indexBufferUsage);
			bufferData.bufferSizes[MeshBufferData::IndexBuffer] = vdata.vertexDataSize;
		}

		// Bind and update vertex buffer
		renderDevice->BindBuffer(RenderBufferTarget::VertexBuffer, bufferData.bufferObjects[MeshBufferData::VertexBuffer]);

		if (vdata.vertexDataSize <= bufferData.bufferSizes[MeshBufferData::VertexBuffer])
		{
			// Only update the part of the buffer we need
			renderDevice->SetBufferSubData(RenderBufferTarget::VertexBuffer,
				0, vdata.vertexDataSize, vdata.vertexData);
		}
		else
		{
			// SetBufferData reallocates storage when needed
			renderDevice->SetBufferData(RenderBufferTarget::VertexBuffer,
				vdata.vertexDataSize, vdata.vertexData, vdata.vertexBufferUsage);
			bufferData.bufferSizes[MeshBufferData::VertexBuffer] = vdata.vertexDataSize;
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

void MeshManager::CreateDrawData(unsigned int index, const VertexData& vdata)
{
	assert(vdata.vertexCount != 0);

	MeshDrawData& drawData = data.drawData[index];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[index].vertexArrayObject;
	drawData.count = static_cast<int>(vdata.vertexCount);
	drawData.indexType = RenderIndexType::None;

	data.uniqueVertexCount[index] = static_cast<int>(vdata.vertexCount);
}

void MeshManager::CreateDrawDataIndexed(unsigned int index, const IndexedVertexData& vdata)
{
	assert(vdata.vertexCount != 0);

	MeshDrawData& drawData = data.drawData[index];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[index].vertexArrayObject;
	drawData.count = static_cast<int>(vdata.indexCount);
	drawData.indexType = RenderIndexType::UnsignedShort;

	data.uniqueVertexCount[index] = static_cast<int>(vdata.vertexCount);
}

void MeshManager::SetVertexAttribPointers(const VertexFormat& vertexFormat)
{
	assert(vertexFormat.attributes != nullptr && vertexFormat.attributeCount > 0);

	for (unsigned int i = 0; i < vertexFormat.attributeCount; ++i)
	{
		const VertexAttribute& attr = vertexFormat.attributes[i];

		renderDevice->EnableVertexAttribute(attr.attrIndex);

		RenderCommandData::SetVertexAttributePointer data{
			attr.attrIndex, attr.elemCount, attr.elemType, attr.stride, attr.offset
		};

		renderDevice->SetVertexAttributePointer(&data);
	}
}

void MeshManager::Upload(MeshId id, const VertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.vertexDataSize != 0 && vdata.vertexCount != 0);

	unsigned int index = GetIndex(id);

	UpdateBuffers(index, vdata);
	CreateDrawData(index, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}

void MeshManager::UploadIndexed(MeshId id, const IndexedVertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.indexDataSize != 0 && vdata.indexCount != 0);

	unsigned int index = GetIndex(id);

	UpdateBuffersIndexed(index, vdata);
	CreateDrawDataIndexed(index, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}
