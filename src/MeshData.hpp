#pragma once

enum class MeshPrimitiveMode
{
	Points,
	LineStrip,
	LineLoop,
	Lines,
	TriangleStrip,
	TriangleFan,
	Triangles
};

template<typename VertexType, typename IndexType>
struct IndexedVertexData
{
	MeshPrimitiveMode primitiveMode;

	VertexType* vertData;
	unsigned int vertCount;
	IndexType* idxData;
	unsigned int idxCount;
};

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }
};
