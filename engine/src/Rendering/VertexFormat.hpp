#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"

#include "Rendering/RenderTypes.hpp"

struct VertexAttribute
{
	VertexAttribute() = default;

	VertexAttribute(unsigned int attrIndex, int elemCount) :
		attrIndex(attrIndex),
		elemCount(elemCount)
	{
	}

	unsigned int attrIndex = 0;
	int elemCount = 0;
	uintptr_t offset = 0;
	int stride = 0;
	RenderVertexElemType elemType = RenderVertexElemType::Float;

	static VertexAttribute pos2;
	static VertexAttribute pos3;
	static VertexAttribute pos4;
	static VertexAttribute nor;
	static VertexAttribute rgb0;
	static VertexAttribute rgba0;
	static VertexAttribute rgb1;
	static VertexAttribute rgba1;
	static VertexAttribute rgb2;
	static VertexAttribute rgba2;
	static VertexAttribute uv0;
	static VertexAttribute uvw0;
	static VertexAttribute uv1;
	static VertexAttribute uvw1;
	static VertexAttribute uv2;
	static VertexAttribute uvw2;

	static const VertexAttribute& GetPositionAttribute(unsigned componentCount);
	static const VertexAttribute& GetColorAttribute(unsigned int index, unsigned componentCount);
	static const VertexAttribute& GetTextureCoordAttribute(unsigned int index, unsigned componentCount);
};

struct VertexFormat
{
	VertexFormat() = default;
	VertexFormat(VertexAttribute* attributes, unsigned int attributeCount) :
		attributes(attributes),
		attributeCount(attributeCount)
	{
	}

	VertexAttribute* attributes = nullptr;
	unsigned int attributeCount = 0;

	void CalcOffsetsAndSizeInterleaved()
	{
		unsigned int size = 0;

		for (unsigned int i = 0; i < attributeCount; ++i)
		{
			attributes[i].offset = size;
			size += attributes[i].elemCount * sizeof(float);
		}

		for (unsigned int i = 0; i < attributeCount; ++i)
		{
			attributes[i].stride = static_cast<int>(size);
		}
	}

	enum AttributeIndex
	{
		AttributeIndexPos,
		AttributeIndexNor,
		AttributeIndexCol0,
		AttributeIndexCol1,
		AttributeIndexCol2,
		AttributeIndexUV0,
		AttributeIndexUV1,
		AttributeIndexUV2
	};
};
