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
	cmdBuffer(nullptr)
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

		if (bytesProcessed == 0)
		{
			assert(false && "Unrecognized command type");
			KK_LOG_ERROR("Unrecognized command type: {}", static_cast<uint32_t>(type));
			break;
		}

		commandOffset += static_cast<uint32_t>(bytesProcessed);
	}
}

size_t CommandExecutorOpenGL::ParseCommand(RenderCommandType type, const uint8_t* commandBegin)
{
	switch (type)
	{
	// ======================
	// ==== DEBUG GROUPS ====
	// ======================

	case RenderCommandType::PushDebugGroup:
	{
		auto cmd = reinterpret_cast<const CmdPushDebugGroup*>(commandBegin);
		auto message = reinterpret_cast<const char*>(&cmdBuffer->commandData[cmd->messageOffset]);
		glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, cmd->id, cmd->messageLength, message);
		return sizeof(*cmd);
	}

	case RenderCommandType::PopDebugGroup:
		glPopDebugGroup();
		return sizeof(Command);

	// ======================
	// ==== BIND BUFFERS ====
	// ======================

	case RenderCommandType::BindBuffer:
	{
		auto cmd = reinterpret_cast<const CmdBindBuffer*>(commandBegin);
		glBindBuffer(ConvertBufferTarget(cmd->target), cmd->buffer.i);
		return sizeof(*cmd);
	}

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

	// ===========================
	// ==== CLEAR FRAMEBUFFER ====
	// ===========================

	case RenderCommandType::Clear:
	{
		auto cmd = reinterpret_cast<const CmdClear*>(commandBegin);
		GLbitfield mask = 0;
		mask |= cmd->mask.color ? GL_COLOR_BUFFER_BIT : 0;
		mask |= cmd->mask.depth ? GL_DEPTH_BUFFER_BIT : 0;
		mask |= cmd->mask.stencil ? GL_STENCIL_BUFFER_BIT : 0;
		glClear(mask);
		return sizeof(*cmd);
	}

	case RenderCommandType::SetClearColor:
	{
		auto cmd = reinterpret_cast<const CmdSetClearColor*>(commandBegin);
		glClearColor(cmd->color.x, cmd->color.y, cmd->color.z, cmd->color.w);
		return sizeof(*cmd);
	}

	case RenderCommandType::SetClearDepth:
	{
		auto cmd = reinterpret_cast<const CmdSetClearDepth*>(commandBegin);
		glClearDepth(cmd->depth);
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
	{
		auto cmd = reinterpret_cast<const CmdDispatchComputeIndirect*>(commandBegin);
		glDispatchComputeIndirect(cmd->offset);
		return sizeof(*cmd);
	}

	// =======================
	// ==== DRAW COMMANDS ====
	// =======================

	case RenderCommandType::Draw:
	{
		auto cmd = reinterpret_cast<const CmdDraw*>(commandBegin);
		glDrawArrays(ConvertPrimitiveMode(cmd->mode), cmd->offset, cmd->vertexCount);
		return sizeof(*cmd);
	}
	
	case RenderCommandType::DrawIndexed:
	{
		auto cmd = reinterpret_cast<const CmdDrawIndexed*>(commandBegin);
		glDrawElements(ConvertPrimitiveMode(cmd->mode), cmd->indexCount, ConvertIndexType(cmd->indexType), nullptr);
		return sizeof(*cmd);
	}

	case RenderCommandType::DrawInstanced:
	{
		auto cmd = reinterpret_cast<const CmdDrawInstanced*>(commandBegin);
		glDrawArraysInstanced(ConvertPrimitiveMode(cmd->mode), cmd->offset, cmd->vertexCount, cmd->instanceCount);
		return sizeof(*cmd);
	}

	case RenderCommandType::DrawIndexedInstanced:
	{
		auto cmd = reinterpret_cast<const CmdDrawIndexedInstanced*>(commandBegin);
		glDrawElementsInstanced(ConvertPrimitiveMode(cmd->mode), cmd->indexCount,
			ConvertIndexType(cmd->indexType), nullptr, cmd->instanceCount);
		return sizeof(*cmd);
	}

	case RenderCommandType::DrawIndirect:
	{
		auto cmd = reinterpret_cast<const CmdDrawIndirect*>(commandBegin);
		glDrawArraysIndirect(ConvertPrimitiveMode(cmd->mode), reinterpret_cast<const void*>(cmd->offset));
		return sizeof(*cmd);
	}

	case RenderCommandType::DrawIndexedIndirect:
	{
		auto cmd = reinterpret_cast<const CmdDrawIndexedIndirect*>(commandBegin);
		glDrawElementsIndirect(ConvertPrimitiveMode(cmd->mode), ConvertIndexType(cmd->indexType),
			reinterpret_cast<const void*>(cmd->offset));
		return sizeof(*cmd);
	}

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

	case RenderCommandType::BindSampler:
	{
		auto cmd = reinterpret_cast<const CmdBindSampler*>(commandBegin);
		glBindSampler(cmd->textureUnit, cmd->sampler.i);
		return sizeof(*cmd);
	}

	// ================
	// ==== SHADER ====
	// ================

	case RenderCommandType::UseShaderProgram:
	{
		auto cmd = reinterpret_cast<const CmdUseShaderProgram*>(commandBegin);
		glUseProgram(cmd->shader.i);
		return sizeof(*cmd);
	}

	// ===============
	// ==== STATE ====
	// ===============

	case RenderCommandType::BlendingEnable:
		glEnable(GL_BLEND);
		return sizeof(Command);

	case RenderCommandType::BlendingDisable:
		glDisable(GL_BLEND);
		return sizeof(Command);

	case RenderCommandType::BlendFunction:
	{
		auto cmd = reinterpret_cast<const CmdBlendFunction*>(commandBegin);
		glBlendFunc(ConvertBlendFactor(cmd->srcFactor), ConvertBlendFactor(cmd->dstFactor));
		return sizeof(*cmd);
	}

	case RenderCommandType::SetCullFace:
	{
		auto cmd = reinterpret_cast<const CmdSetCullFace*>(commandBegin);
		if (cmd->cullFace != RenderCullFace::None)
		{
			glEnable(GL_CULL_FACE);
			glCullFace(ConvertCullFace(cmd->cullFace));
		}
		else
			glDisable(GL_CULL_FACE);
		return sizeof(*cmd);
	}

	case RenderCommandType::DepthTestEnable:
		glEnable(GL_DEPTH_TEST);
		return sizeof(Command);

	case RenderCommandType::DepthTestDisable:
		glDisable(GL_DEPTH_TEST);
		return sizeof(Command);

	case RenderCommandType::SetDepthTestFunction:
	{
		auto cmd = reinterpret_cast<const CmdSetDepthTestFunction*>(commandBegin);
		glDepthFunc(ConvertDepthCompareFunc(cmd->function));
		return sizeof(*cmd);
	}

	case RenderCommandType::DepthWriteEnable:
		glDepthMask(GL_TRUE);
		return sizeof(Command);

	case RenderCommandType::DepthWriteDisable:
		glDepthMask(GL_FALSE);
		return sizeof(Command);

	case RenderCommandType::ScissorTestEnable:
		glEnable(GL_SCISSOR_TEST);
		return sizeof(Command);

	case RenderCommandType::ScissorTestDisable:
		glDisable(GL_SCISSOR_TEST);
		return sizeof(Command);

	case RenderCommandType::SetViewport:
	{
		auto cmd = reinterpret_cast<const CmdSetViewport*>(commandBegin);
		glViewport(cmd->x, cmd->y, cmd->w, cmd->h);
		return sizeof(*cmd);
	}

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
	{
		auto cmd = reinterpret_cast<const CmdBindVertexArray*>(commandBegin);
		glBindVertexArray(cmd->vertexArrayId.i);
		return sizeof(*cmd);
	}

	case RenderCommandType::MemoryBarrier:
	{
		auto cmd = reinterpret_cast<const CmdMemoryBarrier*>(commandBegin);
		GLbitfield flags = 0;
		flags |= cmd->flags.vertexAttribArray ? GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT : 0;
		flags |= cmd->flags.elementArray ? GL_ELEMENT_ARRAY_BARRIER_BIT : 0;
		flags |= cmd->flags.uniform ? GL_UNIFORM_BARRIER_BIT : 0;
		flags |= cmd->flags.textureFetch ? GL_TEXTURE_FETCH_BARRIER_BIT : 0;
		flags |= cmd->flags.shaderImageAccess ? GL_SHADER_IMAGE_ACCESS_BARRIER_BIT : 0;
		flags |= cmd->flags.command ? GL_COMMAND_BARRIER_BIT : 0;
		flags |= cmd->flags.pixelBuffer ? GL_PIXEL_BUFFER_BARRIER_BIT : 0;
		flags |= cmd->flags.textureUpdate ? GL_TEXTURE_UPDATE_BARRIER_BIT : 0;
		flags |= cmd->flags.bufferUpdate ? GL_BUFFER_UPDATE_BARRIER_BIT : 0;
		flags |= cmd->flags.clientMappedBuffer ? GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT : 0;
		flags |= cmd->flags.framebuffer ? GL_FRAMEBUFFER_BARRIER_BIT : 0;
		flags |= cmd->flags.transformFeedback ? GL_TRANSFORM_FEEDBACK_BARRIER_BIT : 0;
		flags |= cmd->flags.atomicCounter ? GL_ATOMIC_COUNTER_BARRIER_BIT : 0;
		flags |= cmd->flags.shaderStorage ? GL_SHADER_STORAGE_BARRIER_BIT : 0;
		flags |= cmd->flags.queryBuffer ? GL_QUERY_BUFFER_BARRIER_BIT : 0;
		glMemoryBarrier(flags);
		return sizeof(*cmd);
	}

	default:
		return 0;
	}
}

} // namespace render
} // namespace kokko