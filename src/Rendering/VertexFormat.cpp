#include "Rendering/VertexFormat.hpp"

#include "System/IncludeOpenGL.hpp"

VertexAttributeInfo VertexFormat_Pos2::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 2)
};
size_t VertexFormat_Pos2::size;

VertexAttributeInfo VertexFormat_Pos3::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3)
};
size_t VertexFormat_Pos3::size;

VertexAttributeInfo VertexFormat_Pos4::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 4)
};
size_t VertexFormat_Pos4::size;

VertexAttributeInfo VertexFormat_Pos3_UV0::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexUV0, 2)
};
size_t VertexFormat_Pos3_UV0::size;

VertexAttributeInfo VertexFormat_Pos3_Nor3::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexNor, 3)
};
size_t VertexFormat_Pos3_Nor3::size;

VertexAttributeInfo VertexFormat_Pos3_Col3::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexCol, 3)
};
size_t VertexFormat_Pos3_Col3::size;

VertexAttributeInfo VertexFormat_Pos3_Nor3_UV0::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexNor, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexUV0, 2)
};
size_t VertexFormat_Pos3_Nor3_UV0::size;

VertexAttributeInfo VertexFormat_Pos3_Nor3_Tan3::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexNor, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexTan, 3)
};
size_t VertexFormat_Pos3_Nor3_Tan3::size;

VertexAttributeInfo VertexFormat_Pos3_Nor3_Col3::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexNor, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexCol, 3)
};
size_t VertexFormat_Pos3_Nor3_Col3::size;

VertexAttributeInfo VertexFormat_Pos3_Nor3_Tan3_UV0::attr[] = {
	VertexAttributeInfo(VertexFormat::AttribIndexPos, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexNor, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexTan, 3),
	VertexAttributeInfo(VertexFormat::AttribIndexUV0, 2)
};
size_t VertexFormat_Pos3_Nor3_Tan3_UV0::size;

static size_t SetSizeAndOffsets(VertexAttributeInfo* attr, size_t count)
{
	size_t size = 0;

	for (size_t i = 0; i < count; ++i)
	{
		attr[i].offset = size;
		size += attr[i].elemCount * 4;
	}

	return size;
}

void VertexFormat::InitializeData()
{
	VertexFormat_Pos2::size = SetSizeAndOffsets(VertexFormat_Pos2::attr, 1);
	VertexFormat_Pos3::size = SetSizeAndOffsets(VertexFormat_Pos3::attr, 1);
	VertexFormat_Pos4::size = SetSizeAndOffsets(VertexFormat_Pos4::attr, 1);
	VertexFormat_Pos3_UV0::size = SetSizeAndOffsets(VertexFormat_Pos3_UV0::attr, 2);
	VertexFormat_Pos3_Nor3::size = SetSizeAndOffsets(VertexFormat_Pos3_Nor3::attr, 2);
	VertexFormat_Pos3_Col3::size = SetSizeAndOffsets(VertexFormat_Pos3_Col3::attr, 2);
	VertexFormat_Pos3_Nor3_UV0::size = SetSizeAndOffsets(VertexFormat_Pos3_Nor3_UV0::attr, 3);
	VertexFormat_Pos3_Nor3_Tan3::size = SetSizeAndOffsets(VertexFormat_Pos3_Nor3_Tan3::attr, 3);
	VertexFormat_Pos3_Nor3_Col3::size = SetSizeAndOffsets(VertexFormat_Pos3_Nor3_Col3::attr, 3);
	VertexFormat_Pos3_Nor3_Tan3_UV0::size = SetSizeAndOffsets(VertexFormat_Pos3_Nor3_Tan3_UV0::attr, 4);
}
