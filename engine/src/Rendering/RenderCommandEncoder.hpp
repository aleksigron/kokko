#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/StringView.hpp"

#include "Math/Vec4.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{

namespace render
{

struct CommandBuffer;
struct ResourceMap;

class CommandEncoder
{
public:
	CommandEncoder(Allocator* allocator, CommandBuffer* buffer);

	// Debug groups

	void PushDebugGroup(uint32_t id, kokko::ConstStringView message);
	void PopDebugGroup();

	// Bind buffers

	void BindBuffer(RenderBufferTarget target, RenderBufferId buffer);
	void BindBufferBase(RenderBufferTarget target, uint32_t bindingPoint, RenderBufferId buffer);
	void BindBufferRange(
		RenderBufferTarget target, uint32_t bindingPoint, RenderBufferId buffer, intptr_t offset, size_t length);

	// Clear

	void Clear(ClearMask mask);
	void SetClearColor(const Vec4f& color);
	void SetClearDepth(float depth);

	// Compute

	void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
	void DispatchComputeIndirect(intptr_t offset);

	// Draw

	void Draw(RenderPrimitiveMode mode, int32_t offset, int32_t vertexCount);
	void DrawIndexed(RenderPrimitiveMode mode, int32_t indexCount, RenderIndexType indexType);
	void DrawInstanced(RenderPrimitiveMode mode, int32_t offset, int32_t vertexCount, int32_t instanceCount);
	void DrawIndexedInstanced(RenderPrimitiveMode mode, int32_t indexCount, RenderIndexType indexType, int32_t instanceCount);
	void DrawIndirect(RenderPrimitiveMode mode, intptr_t offset);
	void DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset);

	// Framebuffer

	void BindFramebuffer(RenderFramebufferId framebuffer);

	// Sampler

	void BindSampler(uint32_t textureUnit, RenderSamplerId sampler);

	// Shader

	void UseShaderProgram(RenderShaderId shader);

	// State

	void BlendingEnable();
	void BlendingDisable();
	void BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor);

	void SetCullFace(RenderCullFace cullFace);

	void DepthTestEnable();
	void DepthTestDisable();
	void SetDepthTestFunction(RenderDepthCompareFunc function);
	void DepthWriteEnable();
	void DepthWriteDisable();

	void ScissorTestEnable();
	void ScissorTestDisable();

	void SetViewport(int32_t x, int32_t y, int32_t w, int32_t h);

	// Texture

	void BindTextureToShader(
		int32_t uniformLocation,
		uint32_t textureUnit,
		RenderTextureId texture);

	// Vertex arrays

	void BindVertexArray(RenderVertexArrayId id);

	// Memory barrier

	void MemoryBarrier(const MemoryBarrierFlags& barrier);

private:
	void CopyCommand(const void* ptr, size_t size);
	uint32_t CopyCommandData(const void* ptr, size_t size);

	Allocator* allocator;
	CommandBuffer* buffer;
};

}
}
