#pragma once

#include <cstdint>

#include "Transform.h"
#include "Buffer.h"
#include "VertexFormat.h"
#include "ObjectId.h"

struct RenderObject
{
	ObjectId id;

	ObjectId material;

	Transform transform;
	
	uint32_t vertexArrayObject;
	
	int32_t indexCount;
	uint32_t indexElementType;

	void UploadVertexData_PosCol(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosCol>& vertex);
	
	void UploadVertexData_PosTex(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};