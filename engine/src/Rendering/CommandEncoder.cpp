#include "Rendering/CommandEncoder.hpp"

#include "Rendering/RenderCommand.hpp"
#include "Rendering/RenderCommandBuffer.hpp"
#include "Rendering/RenderResourceMap.hpp"

namespace kokko
{
namespace render
{

CommandEncoder::CommandEncoder(
	Allocator* allocator,
	CommandBuffer* buffer) :
	allocator(allocator),
	buffer(buffer)
{
}

// ======================
// ==== DEBUG GROUPS ====
// ======================

CommandEncoderDebugScope CommandEncoder::CreateDebugScope(uint32_t id, kokko::ConstStringView message)
{
	return CommandEncoderDebugScope(this, id, message);
}

void CommandEncoder::BeginDebugScope(uint32_t id, kokko::ConstStringView message)
{
	uint32_t messageOffset = CopyCommandData(message.str, message.len);

	CmdBeginDebugScope data{
		RenderCommandType::BeginDebugScope,
		id,
		messageOffset,
		message.len
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::EndDebugScope()
{
	Command data{ RenderCommandType::EndDebugScope };
	CopyCommand(&data, sizeof(data));
}

// ===========================
// ==== CLEAR FRAMEBUFFER ====
// ===========================

void CommandEncoder::Clear(ClearMask mask)
{
	CmdClear data{
		RenderCommandType::Clear,
		mask
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetClearColor(const Vec4f& color)
{
	CmdSetClearColor data{
		RenderCommandType::SetClearColor,
		color
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetClearDepth(float depth)
{
	CmdSetClearDepth data{
		RenderCommandType::SetClearDepth,
		depth
	};

	CopyCommand(&data, sizeof(data));
}

// =================
// ==== BUFFERS ====
// =================

void CommandEncoder::BindBuffer(RenderBufferTarget target, RenderBufferId buffer)
{
	CmdBindBuffer data{
		RenderCommandType::BindBuffer,
		target,
		buffer
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BindBufferBase(RenderBufferTarget target, uint32_t bindingPoint, RenderBufferId buffer)
{
	CmdBindBufferBase data{
		RenderCommandType::BindBufferBase,
		target,
		bindingPoint,
		buffer
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BindBufferRange(
	RenderBufferTarget target,
	uint32_t bindingPoint,
	RenderBufferId id,
	intptr_t offset,
	size_t length)
{
	CmdBindBufferRange data{
		RenderCommandType::BindBufferRange,
		target,
		bindingPoint,
		id,
		offset,
		length
	};

	CopyCommand(&data, sizeof(data));
}

// =================
// ==== COMPUTE ====
// =================

void CommandEncoder::DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ)
{
	CmdDispatchCompute dispatch{
		RenderCommandType::DispatchCompute,
		numGroupsX,
		numGroupsY,
		numGroupsZ,
	};

	CopyCommand(&dispatch, sizeof(dispatch));
}

void CommandEncoder::DispatchComputeIndirect(intptr_t offset)
{
	CmdDispatchComputeIndirect dispatch{
		RenderCommandType::DispatchComputeIndirect,
		offset
	};

	CopyCommand(&dispatch, sizeof(dispatch));
}

// =======================
// ==== DRAW COMMANDS ====
// =======================

void CommandEncoder::Draw(RenderPrimitiveMode mode, int32_t offset, int32_t vertexCount)
{
	CmdDraw data{
		RenderCommandType::Draw,
		mode,
		offset,
		vertexCount
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DrawIndexed(
	RenderPrimitiveMode mode,
	RenderIndexType indexType,
	int32_t indexCount,
	intptr_t indexOffset,
	int32_t baseVertex)
{
	CmdDrawIndexed data{
		RenderCommandType::DrawIndexed,
		mode,
		indexType,
		indexOffset,
		indexCount,
		baseVertex
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DrawIndexedInstanced(
	RenderPrimitiveMode mode,
	RenderIndexType indexType,
	int32_t indexCount,
	intptr_t indexOffset,
	int32_t instanceCount,
	int32_t baseVertex,
	uint32_t baseInstance)
{
	CmdDrawIndexedInstanced data{
		RenderCommandType::DrawIndexedInstanced,
		mode,
		indexType,
		indexOffset,
		indexCount,
		baseVertex,
		instanceCount,
		baseInstance
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DrawIndirect(RenderPrimitiveMode mode, intptr_t offset)
{
	CmdDrawIndirect data{
		RenderCommandType::DrawIndirect,
		mode,
		offset
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset)
{
	CmdDrawIndexedIndirect data{
		RenderCommandType::DrawIndexedIndirect,
		mode,
		indexType,
		offset
	};

	CopyCommand(&data, sizeof(data));
}

// =====================
// ==== FRAMEBUFFER ====
// =====================

void CommandEncoder::BindFramebuffer(RenderFramebufferId framebuffer)
{
	CmdBindFramebuffer data{
		RenderCommandType::BindFramebuffer,
		framebuffer,
	};

	CopyCommand(&data, sizeof(data));
}


// =================
// ==== SAMPLER ====
// =================

void CommandEncoder::BindSampler(uint32_t textureUnit, RenderSamplerId sampler)
{
	CmdBindSampler data{
		RenderCommandType::BindSampler,
		textureUnit,
		sampler
	};

	CopyCommand(&data, sizeof(data));
}

// ================
// ==== SHADER ====
// ================

void CommandEncoder::UseShaderProgram(RenderShaderId shader)
{
	CmdUseShaderProgram data{
		RenderCommandType::UseShaderProgram,
		shader
	};

	CopyCommand(&data, sizeof(data));
}

// ===============
// ==== STATE ====
// ===============

void CommandEncoder::BlendingEnable()
{
	Command data{ RenderCommandType::BlendingEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BlendingDisable()
{
	Command data{ RenderCommandType::BlendingDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor)
{
	CmdBlendFunction data{ RenderCommandType::BlendFunction, srcFactor, dstFactor };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetBlendFunctionSeparate(
	uint32_t attachmentIndex,
	RenderBlendFactor srcFactorRgb,
	RenderBlendFactor dstFactorRgb,
	RenderBlendFactor srcFactorAlpha,
	RenderBlendFactor dstFactorAlpha)
{
	CmdSetBlendFunctionSeparate data{
		RenderCommandType::SetBlendFunctionSeparate,
		attachmentIndex,
		srcFactorRgb,
		dstFactorRgb,
		srcFactorAlpha,
		dstFactorAlpha
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetBlendEquation(uint32_t attachmentIndex, RenderBlendEquation equation)
{
	CmdSetBlendEquation data{
		RenderCommandType::SetBlendEquation,
		attachmentIndex,
		equation
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetCullFace(RenderCullFace cullFace)
{
	CmdSetCullFace data{ RenderCommandType::SetCullFace, cullFace };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthTestEnable()
{
	Command data{ RenderCommandType::DepthTestEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthTestDisable()
{
	Command data{ RenderCommandType::DepthTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetDepthTestFunction(RenderDepthCompareFunc function)
{
	CmdSetDepthTestFunction data{ RenderCommandType::SetDepthTestFunction, function };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthWriteEnable()
{
	Command data{ RenderCommandType::DepthWriteEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthWriteDisable()
{
	Command data{ RenderCommandType::DepthWriteDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::StencilTestDisable()
{
	Command data{ RenderCommandType::StencilTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::ScissorTestEnable()
{
	Command data{ RenderCommandType::ScissorTestEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::ScissorTestDisable()
{
	Command data{ RenderCommandType::ScissorTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetScissorRectangle(int32_t x, int32_t y, int32_t w, int32_t h)
{
	CmdSetScissorRectangle data{ RenderCommandType::SetScissorRectangle, x, y, w, h };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
	CmdSetViewport data{ RenderCommandType::SetViewport, x, y, w, h };
	CopyCommand(&data, sizeof(data));
}

// =================
// ==== TEXTURE ====
// =================

void CommandEncoder::BindTextureToShader(
	int32_t uniformLocation,
	uint32_t textureUnit,
	RenderTextureId texture)
{
	CmdBindTextureToShader data{
		RenderCommandType::BindTextureToShader,
		uniformLocation,
		textureUnit,
		texture
	};

	CopyCommand(&data, sizeof(data));
}

// ======================
// ==== VERTEX ARRAY ====
// ======================

void CommandEncoder::BindVertexArray(RenderVertexArrayId id)
{
	CmdBindVertexArray data{
		RenderCommandType::BindVertexArray,
		id
	};

	CopyCommand(&data, sizeof(data));
}

// ========================
// ==== MEMORY BARRIER ====
// ========================

void CommandEncoder::MemoryBarrier(const MemoryBarrierFlags& barrier)
{
	CmdMemoryBarrier data{
		RenderCommandType::MemoryBarrier,
		barrier
	};

	CopyCommand(&data, sizeof(data));
}

// ===========================
// ==== COPY COMMAND DATA ====
// ===========================

void CommandEncoder::CopyCommand(const void* ptr, size_t size)
{
	size_t offset = buffer->commands.GetCount();
	buffer->commands.Resize(offset + size);
	std::memcpy(&buffer->commands[offset], ptr, size);
}

uint32_t CommandEncoder::CopyCommandData(const void* ptr, size_t size)
{
	uint32_t offset = static_cast<uint32_t>(buffer->commandData.GetCount());
	buffer->commandData.Resize(offset + size);

	std::memcpy(&buffer->commandData[offset], ptr, size);

	return offset;
}

} // namespace render
} // namespace kokko
