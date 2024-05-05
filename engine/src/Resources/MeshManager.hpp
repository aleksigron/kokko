#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Math/AABB.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshId.hpp"

class Allocator;

namespace kokko
{
class AssetLoader;
class Filesystem;

namespace render
{
class Device;
}

struct VertexData
{
	VertexData() :
		primitiveMode(RenderPrimitiveMode::Triangles),
		vertexData(nullptr),
		vertexDataSize(0),
		vertexCount(0)
	{
	}

	VertexFormat vertexFormat;

	RenderPrimitiveMode primitiveMode;

	const void* vertexData;
	size_t vertexDataSize;
	uint32_t vertexCount;
};

struct IndexedVertexData : VertexData
{
	IndexedVertexData() :
		indexData(nullptr),
		indexDataSize(0),
		indexCount(0),
		indexType(RenderIndexType::UnsignedShort)
	{
	}

	const void* indexData;
	size_t indexDataSize;
	uint32_t indexCount;
	RenderIndexType indexType;
};

struct MeshDrawData
{
	kokko::render::VertexArrayId vertexArrayObject;
	int count;
	RenderPrimitiveMode primitiveMode;

	// If indexType is None, this is not an indexed mesh
	RenderIndexType indexType;
};

struct MeshBufferData
{
	enum BufferType { VertexBuffer = 0, IndexBuffer = 1 };

	kokko::render::VertexArrayId vertexArrayObject;
	kokko::render::BufferId bufferObjects[2];
	uint32_t bufferSizes[2];
};

class MeshManager
{
private:
	Allocator* allocator;
	kokko::AssetLoader* assetLoader;
	kokko::render::Device* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void* buffer;

		// Linked list of freed indices
		// This could be combined with index list, since the same index is not used
		// in both at the same time. They are separate for debugging purposes.
		unsigned int* freeList;

		// Mesh ID is used to look into this array
		// Values in this array are used to look into the mesh data arrays below
		// Value 0 means the index is not used
		unsigned int* indexList;

		// Mesh data arrays
		// The items can be reordered and compacted because of the indexList indirection

		MeshDrawData* drawData;
		MeshBufferData* bufferData;
		int* uniqueVertexCount; // Used for rendering normal visualization
		AABB* bounds;
		MeshId* meshId;
		Uid* uid;
		bool* uidExists;
	}
	data;

	unsigned int freeListFirst;
	HashMap<Uid, MeshId> uidMap;

	void Reallocate(unsigned int required);

	void UpdateBuffers(unsigned int index, const VertexData& vdata);
	void UpdateBuffersIndexed(unsigned int index, const IndexedVertexData& vdata);

	void DeleteBuffers(MeshBufferData& buffers) const;

	void CreateDrawData(unsigned int index, const VertexData& vdata);
	void CreateDrawDataIndexed(unsigned int index, const IndexedVertexData& vdata);

	void SetVertexAttribPointers(unsigned int index, const VertexFormat& vertexFormat);

	unsigned int GetIndex(MeshId meshId) const;

public:
	MeshManager(Allocator* allocator, AssetLoader* assetLoader, render::Device* renderDevice);
	~MeshManager();

	//MeshId CreateMesh();
	//void RemoveMesh(MeshId id);

	Optional<Uid> GetUid(MeshId id) const;
	void SetUid(MeshId id, const Uid& uid);

	const AABB* GetBoundingBox(MeshId id) const;
	void SetBoundingBox(MeshId id, const AABB& bounds);

	const MeshDrawData* GetDrawData(MeshId id) const;
	int GetUniqueVertexCount(MeshId id) const;

	void Upload(MeshId id, const VertexData& vdata);
	void UploadIndexed(MeshId id, const IndexedVertexData& vdata);
};

} // namespace kokko
