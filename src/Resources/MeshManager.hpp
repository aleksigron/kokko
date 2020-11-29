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

struct VertexData
{
	VertexData() :
		primitiveMode(RenderPrimitiveMode::Triangles),
		usage(RenderBufferUsage::StaticDraw),
		vertexData(nullptr),
		vertexCount(0)
	{
	}

	VertexFormat vertexFormat;

	RenderPrimitiveMode primitiveMode;
	RenderBufferUsage usage;

	unsigned int vertexCount;
	const void* vertexData;
};

struct IndexedVertexData : VertexData
{
	IndexedVertexData() :
		indexData(nullptr),
		indexCount(0),
		indexSize(sizeof(unsigned short))
	{
	}

	void SetIndexSizeShort() { indexSize = sizeof(unsigned short); }
	void SetIndexSizeInt() { indexSize = sizeof(unsigned int); }

	const void* indexData;
	unsigned int indexCount;

	unsigned int indexSize;
};

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

	void UpdateBuffers(MeshId id, const void* vertBuf, unsigned int vertBytes, RenderBufferUsage usage);
	void UpdateIndexedBuffers(MeshId id, const void* vertBuf, unsigned int vertBytes,
		const void* idxBuf, unsigned int idxBytes, RenderBufferUsage usage);

	void DeleteBuffers(MeshBufferData& buffers) const;

	void CreateDrawData(MeshId id, const VertexData& vdata);
	void CreateDrawDataIndexed(MeshId id, const IndexedVertexData& vdata);

	void SetVertexAttribPointers(const VertexFormat& vertexFormat);

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

	void Upload(MeshId id, const VertexData& vdata);
	void UploadIndexed(MeshId id, const IndexedVertexData& vdata);

};
