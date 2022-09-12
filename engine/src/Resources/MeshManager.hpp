#pragma once

#include <cstdint>

#include "Core/HashMap.hpp"
#include "Core/Optional.hpp"
#include "Core/StringView.hpp"
#include "Core/Uid.hpp"

#include "Math/BoundingBox.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/VertexFormat.hpp"

#include "Resources/MeshId.hpp"

namespace kokko
{
class AssetLoader;
}

struct BoundingBox;

class Allocator;
class Filesystem;
class RenderDevice;

struct VertexData
{
	VertexData() :
		vertexBufferUsage(RenderBufferUsage::StaticDraw),
		primitiveMode(RenderPrimitiveMode::Triangles),
		vertexData(nullptr),
		vertexDataSize(0),
		vertexCount(0)
	{
	}

	VertexFormat vertexFormat;
	RenderBufferUsage vertexBufferUsage;

	RenderPrimitiveMode primitiveMode;

	const void* vertexData;
	unsigned int vertexDataSize;

	unsigned int vertexCount;
};

struct IndexedVertexData : VertexData
{
	IndexedVertexData() :
		indexBufferUsage(RenderBufferUsage::StaticDraw),
		indexData(nullptr),
		indexDataSize(0),
		indexCount(0)
	{
	}

	RenderBufferUsage indexBufferUsage;

	const void* indexData;
	unsigned int indexDataSize;

	unsigned int indexCount;
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
	kokko::AssetLoader* assetLoader;
	RenderDevice* renderDevice;

	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

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
		BoundingBox* bounds;
		MeshId* meshId;
		kokko::Uid* uid;
		bool* uidExists;
	}
	data;

	unsigned int freeListFirst;
	HashMap<kokko::Uid, MeshId> uidMap;

	void Reallocate(unsigned int required);

	void UpdateBuffers(unsigned int index, const VertexData& vdata);
	void UpdateBuffersIndexed(unsigned int index, const IndexedVertexData& vdata);

	void DeleteBuffers(MeshBufferData& buffers) const;

	void CreateDrawData(unsigned int index, const VertexData& vdata);
	void CreateDrawDataIndexed(unsigned int index, const IndexedVertexData& vdata);

	void SetVertexAttribPointers(const VertexFormat& vertexFormat);

	unsigned int GetIndex(MeshId meshId) const;

public:
	MeshManager(Allocator* allocator, kokko::AssetLoader* assetLoader, RenderDevice* renderDevice);
	~MeshManager();

	MeshId CreateMesh();
	void RemoveMesh(MeshId id);

	Optional<kokko::Uid> GetUid(MeshId id) const;
	void SetUid(MeshId id, const kokko::Uid& uid);

	const BoundingBox* GetBoundingBox(MeshId id) const;
	void SetBoundingBox(MeshId id, const BoundingBox& bounds);

	const MeshDrawData* GetDrawData(MeshId id) const;
	int GetUniqueVertexCount(MeshId id) const;

	void Upload(MeshId id, const VertexData& vdata);
	void UploadIndexed(MeshId id, const IndexedVertexData& vdata);
};
