#include "Rendering/RenderCommandExecutorOpenGL.hpp"

#include <cassert>

#include "System/IncludeOpenGL.hpp"

#include "Rendering/RenderCommandBuffer.hpp"
#include "Rendering/RenderCommand.hpp"
#include "Rendering/RenderDeviceEnumsOpenGL.hpp"
#include "Rendering/RenderResourceMap.hpp"

namespace kokko
{

namespace render
{

CommandExecutorOpenGL::CommandExecutorOpenGL(Allocator* allocator) :
	cmdBuffer(nullptr),
	tempIds(allocator)
{
}

void CommandExecutorOpenGL::Execute(const CommandBuffer* commandBuffer)
{
	cmdBuffer = commandBuffer;
	uint32_t commandOffset = 0;

	uint32_t end = static_cast<uint32_t>(cmdBuffer->commands.GetCount());
	while (commandOffset < end)
	{
		const uint8_t* commandBegin = &cmdBuffer->commands[commandOffset];
		RenderCommandType type = *reinterpret_cast<const RenderCommandType*>(commandBegin);
		size_t bytesProcessed = ParseCommand(type, commandBegin);
		commandOffset += static_cast<uint32_t>(bytesProcessed);
	}
}

size_t CommandExecutorOpenGL::ParseCommand(RenderCommandType type, const uint8_t* commandBegin)
{
	switch (type)
	{
		// =================
		// ==== BUFFERS ====
		// =================

	case RenderCommandType::BindBufferBase:
	{
		auto cmd = reinterpret_cast<const CmdBindBufferBase*>(commandBegin);
		glBindBufferBase(ConvertBufferTarget(cmd->target), cmd->bindingPoint, cmd->buffer.i);
		return sizeof(*cmd);
	}

	case RenderCommandType::BindBufferRange:
	{
		auto cmd = reinterpret_cast<const CmdBindBufferRange*>(commandBegin);
		glBindBufferRange(ConvertBufferTarget(cmd->target), cmd->bindingPoint, cmd->buffer.i, cmd->offset, cmd->length);
		return sizeof(*cmd);
	}

	// =================
	// ==== COMPUTE ====
	// =================

	case RenderCommandType::DispatchCompute:
	{
		auto cmd = reinterpret_cast<const CmdDispatchCompute*>(commandBegin);

		glDispatchCompute(cmd->numGroupsX, cmd->numGroupsY, cmd->numGroupsZ);

		return sizeof(*cmd);
	}
	case RenderCommandType::DispatchComputeIndirect:
		break;

		// =======================
		// ==== DRAW COMMANDS ====
		// =======================

	case RenderCommandType::Draw:
		break;
	case RenderCommandType::DrawIndexed:
		break;
	case RenderCommandType::DrawInstanced:
		break;
	case RenderCommandType::DrawIndexedInstanced:
		break;
	case RenderCommandType::DrawIndirect:
		break;
	case RenderCommandType::DrawIndexedIndirect:
		break;

		// =====================
		// ==== FRAMEBUFFER ====
		// =====================

	case RenderCommandType::BindFramebuffer:
	{
		auto cmd = reinterpret_cast<const CmdBindFramebuffer*>(commandBegin);
		glBindFramebuffer(GL_FRAMEBUFFER, cmd->framebuffer.i);

		return sizeof(*cmd);
	}

	// =================
	// ==== SAMPLER ====
	// =================

	// ================
	// ==== SHADER ====
	// ================

	// ===============
	// ==== STATE ====
	// ===============

	// =================
	// ==== TEXTURE ====
	// =================

	case RenderCommandType::BindTextureToShader:
	{
		auto cmd = reinterpret_cast<const CmdBindTextureToShader*>(commandBegin);
		glBindTextureUnit(cmd->textureUnit, cmd->texture.i);
		glUniform1i(cmd->uniformLocation, cmd->textureUnit);
		return sizeof(*cmd);
	}

	// ======================
	// ==== VERTEX ARRAY ====
	// ======================

	case RenderCommandType::BindVertexArray:
		break;
	default:
		assert(false && "Unsupported command type");
		break;
	}
}

} // namespace render
} // namespace kokko