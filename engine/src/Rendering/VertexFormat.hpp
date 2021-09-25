#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

struct VertexAttribute
{
	VertexAttribute() :
		attrIndex(0),
		elemCount(0),
		offset(0),
		elemType(RenderVertexElemType::Float)
	{
	}

	VertexAttribute(unsigned int attrIndex, int elemCount) :
		attrIndex(attrIndex),
		elemCount(elemCount),
		offset(0),
		elemType(RenderVertexElemType::Float)
	{
	}

	unsigned int attrIndex;
	int elemCount;
	uintptr_t offset;
	RenderVertexElemType elemType;

	static VertexAttribute pos2;
	static VertexAttribute pos3;
	static VertexAttribute pos4;
	static VertexAttribute nor;
	static VertexAttribute tan;
	static VertexAttribute bit;
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
	VertexFormat() :
		attributes(nullptr),
		attributeCount(0),
		vertexSize(0)
	{
	}

	VertexFormat(VertexAttribute* attributes, unsigned int attributeCount) :
		attributes(attributes),
		attributeCount(attributeCount),
		vertexSize(0)
	{
		CalculateSizeAndOffsets();
	}

	VertexAttribute* attributes;
	unsigned int attributeCount;
	unsigned int vertexSize;

	void CalculateSizeAndOffsets()
	{
		unsigned int size = 0;

		for (unsigned int i = 0; i < attributeCount; ++i)
		{
			attributes[i].offset = size;
			size += attributes[i].elemCount * sizeof(float);
		}

		vertexSize = size;
	}

	enum AttributeIndex
	{
		AttributeIndexPos,
		AttributeIndexNor,
		AttributeIndexTan,
		AttributeIndexBit,
		AttributeIndexCol0,
		AttributeIndexCol1,
		AttributeIndexCol2,
		AttributeIndexUV0,
		AttributeIndexUV1,
		AttributeIndexUV2
	};
};
