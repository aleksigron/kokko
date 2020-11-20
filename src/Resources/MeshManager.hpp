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
	unsigned int primitiveMode;
	int count;

	// If indexType is 0, this is not an indexed mesh
	unsigned int indexType;
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

	MeshBufferData CreateIndexedBuffers(
		const void* vd, unsigned int vs, const void* id, unsigned int is,
		RenderData::BufferUsage usage);

	MeshBufferData CreateBuffers(
		const void* vd, unsigned int vs,
		RenderData::BufferUsage usage);

	void UpdateIndexedBuffers(MeshBufferData& bufferDataInOut,
		const void* vd, unsigned int vs, const void* id, unsigned int is,
		RenderData::BufferUsage usage);

	void UpdateBuffers(MeshBufferData& bufferDataInOut,
		const void* vd, unsigned int vs,
		RenderData::BufferUsage usage);

	void DeleteBuffers(MeshBufferData& buffers) const;

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

	void Upload_3f(MeshId id, VertexData<Vertex3f> vdata, RenderData::BufferUsage usage);
	void UploadIndexed_3f(MeshId id, IndexedVertexData<Vertex3f, unsigned short> data, RenderData::BufferUsage usage);
	void Upload_3f2f(MeshId id, IndexedVertexData<Vertex3f2f, unsigned short> data, RenderData::BufferUsage usage);
	void Upload_3f3f(MeshId id, IndexedVertexData<Vertex3f3f, unsigned short> data, RenderData::BufferUsage usage);
	void Upload_3f3f2f(MeshId id, IndexedVertexData<Vertex3f3f2f, unsigned short> data, RenderData::BufferUsage usage);
	void Upload_3f3f3f(MeshId id, IndexedVertexData<Vertex3f3f3f, unsigned short> data, RenderData::BufferUsage usage);
};
