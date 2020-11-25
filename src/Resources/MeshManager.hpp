#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/BufferRef.hpp"
#include "Core/StringRef.hpp"

#include "Math/BoundingBox.hpp"

#include "Rendering/RenderDeviceEnums.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshData.hpp"

struct BoundingBox;
class Allocator;
class RenderDevice;

struct MeshDrawData
{
	unsigned int vertexArrayObject;
	int count;
	RenderPrimitiveMode primitiveMode;

	// If indexType is None, this is not an indexed mesh
	RenderIndexType indexType;
};

struct MeshBufferData
{
	enum BufferType { VertexBuffer = 0, IndexBuffer = 1 };

	unsigned int vertexArrayObject;
	unsigned int bufferObjects[2];
	unsigned int bufferSizes[2];
};

class MeshManager
{
private:
	Allocator* allocator;

	RenderDevice* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		unsigned int* freeList;
		MeshDrawData* drawData;
		MeshBufferData* bufferData;
		BoundingBox* bounds;
	}
	data;

	unsigned int freeListFirst;
	HashMap<uint32_t, MeshId> nameHashMap;

	void Reallocate(unsigned int required);

	void UpdateIndexedBuffers(MeshId id, const void* vdata, unsigned int vsize,
		const void* idata, unsigned int isize, RenderBufferUsage usage);

	void UpdateBuffers(MeshId id, const void* vdata, unsigned int vsize, RenderBufferUsage usage);

	void DeleteBuffers(MeshBufferData& buffers) const;

	void CreateDrawData(MeshId id, const VertexData& vdata);
	void CreateIndexedDrawData(MeshId id, const IndexedVertexData<unsigned short>& vdata);

	void SetVertexAttribPointers(int stride, unsigned int count, const VertexAttributeInfo* attributes);

public:
	MeshManager(Allocator* allocator, RenderDevice* renderDevice);
	~MeshManager();

	MeshId CreateMesh();
	void RemoveMesh(MeshId id);
	
	MeshId GetIdByPath(StringRef path);
	MeshId GetIdByPathHash(uint32_t pathHash)
	{
		auto pair = nameHashMap.Lookup(pathHash);
		return pair != nullptr ? pair->second : MeshId{};
	}

	BoundingBox* GetBoundingBox(MeshId id) { return data.bounds + id.i; }
	void SetBoundingBox(MeshId id, const BoundingBox& bounds) { data.bounds[id.i] = bounds; }

	MeshDrawData* GetDrawData(MeshId id) { return data.drawData + id.i; }

	MeshBufferData* GetBufferData(MeshId id) { return data.bufferData + id.i; }

	void Upload_Pos3(MeshId id, VertexData vdata, RenderBufferUsage usage);
	void UploadIndexed_Pos3(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
	void UploadIndexed_Pos3_UV0(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
	void UploadIndexed_Pos3_Nor(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
	void UploadIndexed_Pos3_Col(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
	void UploadIndexed_Pos3_Nor_UV0(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
	void UploadIndexed_Pos3_Nor_Col(MeshId id, IndexedVertexData<unsigned short> data, RenderBufferUsage usage);
};
