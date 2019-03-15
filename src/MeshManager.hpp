#pragma once

#include <cstdint>

#include "MeshData.hpp"

#include "HashMap.hpp"
#include "BoundingBox.hpp"
#include "VertexFormat.hpp"
#include "BufferRef.hpp"
#include "StringRef.hpp"

struct BoundingBox;


struct MeshDrawData
{
	unsigned int vertexArrayObject;
	int indexCount;
	unsigned int indexElementType;
	unsigned int primitiveMode;
};

struct MeshBufferData
{
	enum BufferType { VertexBuffer = 0, IndexBuffer = 1 };

	unsigned int vertexArrayObject;
	unsigned int bufferObjects[2];
};

class MeshManager
{
private:
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

public:
	MeshManager();
	~MeshManager();

	MeshId CreateMesh();
	void RemoveMesh(MeshId id);
	
	MeshId GetIdByPath(StringRef path);
	MeshId GetIdByPathHash(uint32_t pathHash)
	{
		auto pair = nameHashMap.Lookup(pathHash);
		return pair != nullptr ? pair->value : MeshId{};
	}

	BoundingBox* GetBoundingBox(MeshId id) { return data.bounds + id.i; }
	void SetBoundingBox(MeshId id, const BoundingBox& bounds) { data.bounds[id.i] = bounds; }

	MeshDrawData* GetDrawData(MeshId id) { return data.drawData + id.i; }

	MeshBufferData* GetBufferData(MeshId id) { return data.bufferData + id.i; }

	void Upload_3f(MeshId id, IndexedVertexData<Vertex3f, unsigned short> data);
	void Upload_3f2f(MeshId id, IndexedVertexData<Vertex3f2f, unsigned short> data);
	void Upload_3f3f(MeshId id, IndexedVertexData<Vertex3f3f, unsigned short> data);
	void Upload_3f3f2f(MeshId id, IndexedVertexData<Vertex3f3f2f, unsigned short> data);
	void Upload_3f3f3f(MeshId id, IndexedVertexData<Vertex3f3f3f, unsigned short> data);
};
