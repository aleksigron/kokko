#pragma once

#include "Rendering/RenderDeviceEnums.hpp"

template<typename IndexType>
struct IndexedVertexData
{
	RenderPrimitiveMode primitiveMode;

	const float* vertData;
	unsigned int vertCount;
	const IndexType* idxData;
	unsigned int idxCount;
};

struct VertexData
{
	RenderPrimitiveMode primitiveMode;

	const float* vertData;
	unsigned int vertCount;
};

struct MeshId
{
	unsigned int i;

	bool IsValid() const { return i != 0; }
};
