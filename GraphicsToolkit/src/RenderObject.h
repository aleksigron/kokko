#pragma once

#include <cstdint>

#include <OpenGL/gltypes.h>

#include "Transform.h"
#include "Buffer.h"
#include "VertexFormat.h"
#include "ShaderProgram.h"
#include "Texture.h"

struct RenderObjectId
{
	uint32_t index;
	uint32_t innerId;
};

struct RenderObject
{
	RenderObjectId id;

	Transform transform;

	ShaderProgramId shader;
	TextureId texture;
	
	GLuint vertexArrayObject;
	
	GLsizei indexCount;
	GLenum indexElementType;

	bool hasTexture;

	void UploadVertexData_PosCol(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosCol>& vertex);
	
	void UploadVertexData_PosTex(const Buffer<uint16_t>& index,
								 const Buffer<Vertex_PosTex>& vertex);
};