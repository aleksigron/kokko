#pragma once

#include "Rendering/RenderDeviceEnums.hpp"

template<typename VertexType, typename IndexType>
struct IndexedVertexData
{
	RenderPrimitiveMode primitiveMode;

	const VertexType* vertData;
	unsigned int vertCount;
	const IndexType* idxData;
	unsigned int idxCount;
};

template<typename VertexType>
struct VertexData
{
	RenderPrimitiveMode primitiveMode;

	const VertexType* vertData;
	unsigned int vertCount;
};

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }
};
