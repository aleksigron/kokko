#include "Rendering/VertexFormat.hpp"

VertexAttribute VertexAttribute::pos2 = VertexAttribute(VertexFormat::AttributeIndexPos, 2);
VertexAttribute VertexAttribute::pos3 = VertexAttribute(VertexFormat::AttributeIndexPos, 3);
VertexAttribute VertexAttribute::pos4 = VertexAttribute(VertexFormat::AttributeIndexPos, 4);
VertexAttribute VertexAttribute::nor = VertexAttribute(VertexFormat::AttributeIndexNor, 3);
VertexAttribute VertexAttribute::rgb0 = VertexAttribute(VertexFormat::AttributeIndexCol0, 3);
VertexAttribute VertexAttribute::rgba0 = VertexAttribute(VertexFormat::AttributeIndexCol0, 4);
VertexAttribute VertexAttribute::rgb1 = VertexAttribute(VertexFormat::AttributeIndexCol1, 3);
VertexAttribute VertexAttribute::rgba1 = VertexAttribute(VertexFormat::AttributeIndexCol1, 4);
VertexAttribute VertexAttribute::rgb2 = VertexAttribute(VertexFormat::AttributeIndexCol2, 3);
VertexAttribute VertexAttribute::rgba2 = VertexAttribute(VertexFormat::AttributeIndexCol2, 4);
VertexAttribute VertexAttribute::uv0 = VertexAttribute(VertexFormat::AttributeIndexUV0, 2);
VertexAttribute VertexAttribute::uvw0 = VertexAttribute(VertexFormat::AttributeIndexUV0, 3);
VertexAttribute VertexAttribute::uv1 = VertexAttribute(VertexFormat::AttributeIndexUV1, 2);
VertexAttribute VertexAttribute::uvw1 = VertexAttribute(VertexFormat::AttributeIndexUV1, 3);
VertexAttribute VertexAttribute::uv2 = VertexAttribute(VertexFormat::AttributeIndexUV2, 2);
VertexAttribute VertexAttribute::uvw2 = VertexAttribute(VertexFormat::AttributeIndexUV2, 3);

const VertexAttribute& VertexAttribute::GetPositionAttribute(unsigned componentCount)
{
	if (componentCount == 3) return pos3;
	else if (componentCount == 2) return pos2;
	else return pos4;
}

const VertexAttribute& VertexAttribute::GetColorAttribute(unsigned int index, unsigned componentCount)
{
	if (index == 0)
	{
		if (componentCount == 3) return rgb0;
		else return rgba0;
	}
	else if (index == 1)
	{
		if (componentCount == 3) return rgb1;
		else return rgba1;
	}
	else
	{
		if (componentCount == 3) return rgb2;
		else return rgba2;
	}
}

const VertexAttribute& VertexAttribute::GetTextureCoordAttribute(unsigned int index, unsigned componentCount)
{
	if (index == 0)
	{
		if (componentCount == 2) return uv0;
		else return uvw0;
	}
	else if (index == 1)
	{
		if (componentCount == 2) return uv1;
		else return uvw1;
	}
	else
	{
		if (componentCount == 2) return uv2;
		else return uvw2;
	}
}