#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

struct VertexFormat
{
	static constexpr unsigned int AttribIndexPos = 0;
	static constexpr unsigned int AttribIndexNor = 1;
	static constexpr unsigned int AttribIndexTan = 2;
	static constexpr unsigned int AttribIndexCol = 3;
	static constexpr unsigned int AttribIndexUV0 = 4;

	static void InitializeData();
};

struct VertexAttributeInfo
{
	VertexAttributeInfo(unsigned int attrIndex, int elemCount) :
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
};

struct VertexFormat_Pos2
{
	static VertexAttributeInfo attr[1];
	static size_t size;
};

struct VertexFormat_Pos3
{
	static VertexAttributeInfo attr[1];
	static size_t size;
};

struct VertexFormat_Pos4
{
	static VertexAttributeInfo attr[1];
	static size_t size;
};

struct VertexFormat_Pos3_UV0
{
	static VertexAttributeInfo attr[2];
	static size_t size;
};

struct VertexFormat_Pos3_Nor3
{
	static VertexAttributeInfo attr[2];
	static size_t size;
};

struct VertexFormat_Pos3_Col3
{
	static VertexAttributeInfo attr[2];
	static size_t size;
};

struct VertexFormat_Pos3_Nor3_UV0
{
	static VertexAttributeInfo attr[3];
	static size_t size;
};

struct VertexFormat_Pos3_Nor3_Tan3
{
	static VertexAttributeInfo attr[3];
	static size_t size;
};

struct VertexFormat_Pos3_Nor3_Col3
{
	static VertexAttributeInfo attr[3];
	static size_t size;
};

struct VertexFormat_Pos3_Nor3_Tan3_UV0
{
	static VertexAttributeInfo attr[4];
	static size_t size;
};
