#include "Rendering/CommandEncoder.hpp"

#include "Rendering/RenderCommand.hpp"
#include "Rendering/RenderCommandBuffer.hpp"

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
		CommandType::BeginDebugScope,
		id,
		messageOffset,
		message.len
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::EndDebugScope()
{
	Command data{ CommandType::EndDebugScope };
	CopyCommand(&data, sizeof(data));
}

// ===========================
// ==== CLEAR FRAMEBUFFER ====
// ===========================

void CommandEncoder::Clear(ClearMask mask)
{
	CmdClear data{
		CommandType::Clear,
		mask
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetClearColor(const Vec4f& color)
{
	CmdSetClearColor data{
		CommandType::SetClearColor,
		color
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetClearDepth(float depth)
{
	CmdSetClearDepth data{
		CommandType::SetClearDepth,
		depth
	};

	CopyCommand(&data, sizeof(data));
}

// =================
// ==== BUFFERS ====
// =================

void CommandEncoder::BindBuffer(RenderBufferTarget target, render::BufferId buffer)
{
	CmdBindBuffer data{
		CommandType::BindBuffer,
		target,
		buffer
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BindBufferBase(RenderBufferTarget target, uint32_t bindingPoint, render::BufferId buffer)
{
	CmdBindBufferBase data{
		CommandType::BindBufferBase,
		target,
		bindingPoint,
		buffer
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BindBufferRange(
	RenderBufferTarget target,
	uint32_t bindingPoint,
	render::BufferId id,
	intptr_t offset,
	size_t length)
{
	CmdBindBufferRange data{
		CommandType::BindBufferRange,
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
		CommandType::DispatchCompute,
		numGroupsX,
		numGroupsY,
		numGroupsZ,
	};

	CopyCommand(&dispatch, sizeof(dispatch));
}

void CommandEncoder::DispatchComputeIndirect(intptr_t offset)
{
	CmdDispatchComputeIndirect dispatch{
		CommandType::DispatchComputeIndirect,
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
		CommandType::Draw,
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
		CommandType::DrawIndexed,
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
		CommandType::DrawIndexedInstanced,
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
		CommandType::DrawIndirect,
		mode,
		offset
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset)
{
	CmdDrawIndexedIndirect data{
		CommandType::DrawIndexedIndirect,
		mode,
		indexType,
		offset
	};

	CopyCommand(&data, sizeof(data));
}

// =====================
// ==== FRAMEBUFFER ====
// =====================

void CommandEncoder::BindFramebuffer(render::FramebufferId framebuffer)
{
	CmdBindFramebuffer data{
		CommandType::BindFramebuffer,
		framebuffer,
	};

	CopyCommand(&data, sizeof(data));
}


// =================
// ==== SAMPLER ====
// =================

void CommandEncoder::BindSampler(uint32_t textureUnit, render::SamplerId sampler)
{
	CmdBindSampler data{
		CommandType::BindSampler,
		textureUnit,
		sampler
	};

	CopyCommand(&data, sizeof(data));
}

// ================
// ==== SHADER ====
// ================

void CommandEncoder::UseShaderProgram(render::ShaderId shader)
{
	CmdUseShaderProgram data{
		CommandType::UseShaderProgram,
		shader
	};

	CopyCommand(&data, sizeof(data));
}

// ===============
// ==== STATE ====
// ===============

void CommandEncoder::BlendingEnable()
{
	Command data{ CommandType::BlendingEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BlendingDisable()
{
	Command data{ CommandType::BlendingDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor)
{
	CmdBlendFunction data{ CommandType::BlendFunction, srcFactor, dstFactor };
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
		CommandType::SetBlendFunctionSeparate,
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
		CommandType::SetBlendEquation,
		attachmentIndex,
		equation
	};

	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetCullFace(RenderCullFace cullFace)
{
	CmdSetCullFace data{ CommandType::SetCullFace, cullFace };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthTestEnable()
{
	Command data{ CommandType::DepthTestEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthTestDisable()
{
	Command data{ CommandType::DepthTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetDepthTestFunction(RenderDepthCompareFunc function)
{
	CmdSetDepthTestFunction data{ CommandType::SetDepthTestFunction, function };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthWriteEnable()
{
	Command data{ CommandType::DepthWriteEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::DepthWriteDisable()
{
	Command data{ CommandType::DepthWriteDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::StencilTestDisable()
{
	Command data{ CommandType::StencilTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::ScissorTestEnable()
{
	Command data{ CommandType::ScissorTestEnable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::ScissorTestDisable()
{
	Command data{ CommandType::ScissorTestDisable };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetScissorRectangle(int32_t x, int32_t y, int32_t w, int32_t h)
{
	CmdSetScissorRectangle data{ CommandType::SetScissorRectangle, x, y, w, h };
	CopyCommand(&data, sizeof(data));
}

void CommandEncoder::SetViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
	CmdSetViewport data{ CommandType::SetViewport, x, y, w, h };
	CopyCommand(&data, sizeof(data));
}

// =================
// ==== TEXTURE ====
// =================

void CommandEncoder::BindTextureToShader(
	int32_t uniformLocation,
	uint32_t textureUnit,
	render::TextureId texture)
{
	CmdBindTextureToShader data{
		CommandType::BindTextureToShader,
		uniformLocation,
		textureUnit,
		texture
	};

	CopyCommand(&data, sizeof(data));
}

// ======================
// ==== VERTEX ARRAY ====
// ======================

void CommandEncoder::BindVertexArray(render::VertexArrayId id)
{
	CmdBindVertexArray data{
		CommandType::BindVertexArray,
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
		CommandType::MemoryBarrier,
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
