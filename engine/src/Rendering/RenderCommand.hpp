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
	BeginDebugScope,
	EndDebugScope,

	BindBuffer,
	BindBufferBase,
	BindBufferRange,

	Clear,
	SetClearColor,
	SetClearDepth,

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
	SetBlendFunctionSeparate,
	SetBlendEquation,

	SetCullFace,

	DepthTestEnable,
	DepthTestDisable,
	SetDepthTestFunction,
	DepthWriteEnable,
	DepthWriteDisable,

	StencilTestDisable,

	ScissorTestEnable,
	ScissorTestDisable,
	SetScissorRectangle,

	SetViewport,

	BindTextureToShader,

	BindVertexArray,

	MemoryBarrier
};

struct Command
{
	RenderCommandType type;
};

// ======================
// ==== DEBUG GROUPS ====
// ======================

struct CmdBeginDebugScope : public Command
{
	uint32_t id;
	uint32_t messageOffset;
	uint32_t messageLength;
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
	intptr_t indexOffset;
	int32_t indexCount;
	int32_t baseVertex;
};

struct CmdDrawIndexedInstanced : public Command
{
	RenderPrimitiveMode mode;
	RenderIndexType indexType;
	intptr_t indexOffset;
	int32_t indexCount;
	int32_t baseVertex;
	int32_t instanceCount;
	uint32_t baseInstance;
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

struct CmdSetBlendFunctionSeparate : public Command
{
	uint32_t attachmentIndex;
	RenderBlendFactor srcFactorRgb;
	RenderBlendFactor dstFactorRgb;
	RenderBlendFactor srcFactorAlpha;
	RenderBlendFactor dstFactorAlpha;
};

struct CmdSetBlendEquation : public Command
{
	uint32_t attachmentIndex;
	RenderBlendEquation blendEquation;
};

struct CmdSetCullFace : public Command
{
	RenderCullFace cullFace;
};

struct CmdSetDepthTestFunction : public Command
{
	RenderDepthCompareFunc function;
};

struct CmdSetScissorRectangle : public Command
{
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
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

// ======================
// ==== VERTEX ARRAY ====
// ======================

struct CmdMemoryBarrier : public Command
{
	MemoryBarrierFlags flags;
};

}
}