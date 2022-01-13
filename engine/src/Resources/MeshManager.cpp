#include "Resources/MeshManager.hpp"

#include <cassert>

#include "Core/Core.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "Rendering/RenderDevice.hpp"

#include "Resources/AssetLoader.hpp"
#include "Resources/MeshLoader.hpp"

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
	newData.bounds = reinterpret_cast<BoundingBox*>(newData.bufferData + newData.allocated);
	newData.meshId = reinterpret_cast<MeshId*>(newData.bounds + newData.allocated);
	newData.uid = reinterpret_cast<kokko::Uid*>(newData.meshId + newData.allocated);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.freeList, data.freeList, data.allocated * sizeof(unsigned int));
		std::memset(newData.freeList + data.allocated, 0, (newData.allocated - data.allocated) * sizeof(unsigned int));

		std::memcpy(newData.indexList, data.indexList, data.allocated * sizeof(unsigned int));
		std::memset(newData.indexList + data.allocated, 0, (newData.allocated - data.allocated) * sizeof(unsigned int));

		std::memcpy(newData.drawData, data.drawData, data.count * sizeof(MeshDrawData));
		std::memcpy(newData.bufferData, data.bufferData, data.count * sizeof(MeshBufferData));
		std::memcpy(newData.bounds, data.bounds, data.count * sizeof(BoundingBox));
		std::memcpy(newData.meshId, data.meshId, data.count * sizeof(MeshId));
		std::memcpy(newData.uid, data.uid, data.count * sizeof(kokko::Uid));

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

	data.bufferData[index] = MeshBufferData{};
	data.meshId[index] = id;

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
		data.bounds[index] = data.bounds[lastIndex];
		data.meshId[index] = lastMeshId;
		data.uid[index] = data.uid[lastIndex];

		// Update index list
		data.indexList[lastMeshId.i] = index;
	}

	--data.count;
}

MeshId MeshManager::FindModelByUid(const kokko::Uid& uid)
{
	KOKKO_PROFILE_FUNCTION();

	auto* pair = uidMap.Lookup(uid);
	if (pair != nullptr)
		return pair->second;

	if (data.count == data.allocated)
		this->Reallocate(data.count + 1);

	Array<uint8_t> file(allocator);

	if (assetLoader->LoadAsset(uid, file))
	{
		MeshId id = CreateMesh();
		MeshLoader loader(this);

		MeshLoader::Status status = loader.LoadFromBuffer(id, file.GetView());
		if (status == MeshLoader::Status::Success)
		{
			pair = uidMap.Insert(uid);
			pair->second = id;

			data.uid[GetIndex(id)] = uid;

			return id;
		}
		else
		{
			RemoveMesh(id);
		}
	}

	return MeshId{};
}

MeshId MeshManager::FindModelByPath(const StringRef& path)
{
	KOKKO_PROFILE_FUNCTION();

	auto uidResult = assetLoader->GetAssetUidByVirtualPath(path);
	if (uidResult.HasValue())
	{
		return FindModelByUid(uidResult.GetValue());
	}

	return MeshId::Null;
}

unsigned int MeshManager::GetIndex(MeshId meshId) const
{
	unsigned int index = data.indexList[meshId.i];
	assert(index != 0);
	return index;
}

kokko::Uid MeshManager::GetUid(MeshId id) const
{
	return data.uid[GetIndex(id)];
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

const MeshBufferData* MeshManager::GetBufferData(MeshId id) const
{
	return &data.bufferData[GetIndex(id)];
}

void MeshManager::UpdateBuffers(unsigned int index, const void* vertBuf, unsigned int vertBytes, RenderBufferUsage usage)
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
	unsigned int index, const void* vertBuf, unsigned int vertBytes,
	const void* idxBuf, unsigned int idxBytes, RenderBufferUsage usage)
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

void MeshManager::CreateDrawData(unsigned int index, const VertexData& vdata)
{
	MeshDrawData& drawData = data.drawData[index];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[index].vertexArrayObject;
	drawData.count = static_cast<int>(vdata.vertexCount);
	drawData.indexType = RenderIndexType::None;
}

void MeshManager::CreateDrawDataIndexed(unsigned int index, const IndexedVertexData& vdata)
{
	MeshDrawData& drawData = data.drawData[index];
	drawData.primitiveMode = vdata.primitiveMode;
	drawData.vertexArrayObject = data.bufferData[index].vertexArrayObject;
	drawData.count = static_cast<int>(vdata.indexCount);
	drawData.indexType = RenderIndexType::UnsignedShort;
}

void MeshManager::SetVertexAttribPointers(const VertexFormat& vertexFormat)
{
	for (unsigned int i = 0; i < vertexFormat.attributeCount; ++i)
	{
		const VertexAttribute& attr = vertexFormat.attributes[i];

		renderDevice->EnableVertexAttribute(attr.attrIndex);

		int stride = static_cast<int>(vertexFormat.vertexSize);

		RenderCommandData::SetVertexAttributePointer data{
			attr.attrIndex, attr.elemCount, attr.elemType, stride, attr.offset
		};

		renderDevice->SetVertexAttributePointer(&data);
	}
}

void MeshManager::Upload(MeshId id, const VertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.vertexFormat.attributes != nullptr && vdata.vertexFormat.attributeCount > 0);

	unsigned int vsize = vdata.vertexFormat.vertexSize * vdata.vertexCount;

	unsigned int index = GetIndex(id);

	UpdateBuffers(index, vdata.vertexData, vsize, vdata.usage);
	CreateDrawData(index, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}

void MeshManager::UploadIndexed(MeshId id, const IndexedVertexData& vdata)
{
	KOKKO_PROFILE_FUNCTION();

	assert(vdata.vertexFormat.attributes != nullptr && vdata.vertexFormat.attributeCount > 0);

	unsigned int vsize = vdata.vertexFormat.vertexSize * vdata.vertexCount;
	unsigned int isize = vdata.indexSize * vdata.indexCount;

	unsigned int index = GetIndex(id);

	UpdateIndexedBuffers(index, vdata.vertexData, vsize, vdata.indexData, isize, vdata.usage);
	CreateDrawDataIndexed(index, vdata);
	SetVertexAttribPointers(vdata.vertexFormat);
}
