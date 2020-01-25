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

	const VertexType* vertData;
	unsigned int vertCount;
	const IndexType* idxData;
	unsigned int idxCount;
};

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }
};
