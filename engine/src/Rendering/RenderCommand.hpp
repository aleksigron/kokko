#pragma once

#include <cstdint>

#include "RenderTypes.hpp"
#include "RenderResourceId.hpp"

namespace kokko
{

namespace render
{

enum class RenderCommandType : uint16_t
{
	Clear,
	SetClearColor,
	SetClearDepth,

	BindBuffer,
	BindBufferBase,
	BindBufferRange,

	DispatchCompute,
	DispatchComputeIndirect,

	Draw,
	DrawIndexed,
	DrawInstanced,
	DrawIndexedInstanced,
	DrawIndirect,
	DrawIndexedIndirect,

	BindFramebuffer,

	BindSampler,

	UseShaderProgram,

	BlendingEnable,
	BlendingDisable,
	BlendFunction,

	SetCullFace,

	DepthTestEnable,
	DepthTestDisable,
	SetDepthTestFunction,
	DepthWriteEnable,
	DepthWriteDisable,

	ScissorTestEnable,
	ScissorTestDisable,

	SetViewport,

	BindTextureToShader,

	BindVertexArray,
};

struct Command
{
	RenderCommandType type;
};

// ===========================
// ==== CLEAR FRAMEBUFFER ====
// ===========================

struct CmdClear : public Command
{
	ClearMask mask;
};

struct CmdSetClearColor : public Command
{
	Vec4f color;
};

struct CmdSetClearDepth : public Command
{
	float depth;
};

// =================
// ==== BUFFERS ====
// =================

struct CmdBindBuffer : public Command
{
	RenderBufferTarget target;
	RenderBufferId buffer;
};

struct CmdBindBufferBase : public Command
{
	RenderBufferTarget target;
	uint32_t bindingPoint;
	RenderBufferId buffer;
};

struct CmdBindBufferRange : public Command
{
	RenderBufferTarget target;
	uint32_t bindingPoint;
	RenderBufferId buffer;
	intptr_t offset;
	size_t length;
};

// =================
// ==== COMPUTE ====
// =================

struct CmdDispatchCompute : public Command
{
	uint32_t numGroupsX;
	uint32_t numGroupsY;
	uint32_t numGroupsZ;
};

struct CmdDispatchComputeIndirect : public Command
{
	intptr_t offset;
};

// =======================
// ==== DRAW COMMANDS ====
// =======================

struct CmdDraw : public Command
{
	RenderPrimitiveMode mode;
	int32_t offset;
	int32_t vertexCount;
};

struct CmdDrawIndexed : public Command
{
	RenderPrimitiveMode mode;
	RenderIndexType indexType;
	int32_t indexCount;
};

struct CmdDrawInstanced : public Command
{
	RenderPrimitiveMode mode;
	int32_t offset;
	int32_t vertexCount;
	int32_t instanceCount;
};

struct CmdDrawIndexedInstanced : public Command
{
	RenderPrimitiveMode mode;
	RenderIndexType indexType;
	int32_t indexCount;
	int32_t instanceCount;
};

struct CmdDrawIndirect : public Command
{
	RenderPrimitiveMode mode;
	intptr_t offset;
};

struct CmdDrawIndexedIndirect : public Command
{
	RenderPrimitiveMode mode;
	RenderIndexType indexType;
	intptr_t offset;
};

// =====================
// ==== FRAMEBUFFER ====
// =====================

struct CmdBindFramebuffer : public Command
{
	RenderFramebufferId framebuffer;
};

// =================
// ==== SAMPLER ====
// =================

struct CmdBindSampler : public Command
{
	uint32_t textureUnit;
	RenderSamplerId sampler;
};

// ================
// ==== SHADER ====
// ================

struct CmdUseShaderProgram : public Command
{
	RenderShaderId shader;
};

// ===============
// ==== STATE ====
// ===============

struct CmdBlendFunction : public Command
{
	RenderBlendFactor srcFactor;
	RenderBlendFactor dstFactor;
};

struct CmdSetCullFace : public Command
{
	RenderCullFace cullFace;
};

struct CmdSetDepthTestFunction : public Command
{
	RenderDepthCompareFunc function;
};

struct CmdSetViewport : public Command
{
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
};

// =================
// ==== TEXTURE ====
// =================

struct CmdBindTextureToShader : public Command
{
	int32_t uniformLocation;
	uint32_t textureUnit;
	RenderTextureId texture;
};

// ======================
// ==== VERTEX ARRAY ====
// ======================

struct CmdBindVertexArray : public Command
{
	RenderVertexArrayId vertexArrayId;
};

}
}