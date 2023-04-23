#pragma once

#include <cstdint>

#include "Core/Array.hpp"
#include "Core/StringView.hpp"

#include "Math/Vec4.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"
#include "Rendering/CommandEncoderDebugScope.hpp"

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

	// Debug scope

	CommandEncoderDebugScope CreateDebugScope(uint32_t id, kokko::ConstStringView message);
	void BeginDebugScope(uint32_t id, kokko::ConstStringView message);
	void EndDebugScope();

	// Bind buffers

	void BindBuffer(RenderBufferTarget target, render::BufferId buffer);
	void BindBufferBase(RenderBufferTarget target, uint32_t bindingPoint, render::BufferId buffer);
	void BindBufferRange(
		RenderBufferTarget target, uint32_t bindingPoint, render::BufferId buffer, intptr_t offset, size_t length);

	// Clear

	void Clear(ClearMask mask);
	void SetClearColor(const Vec4f& color);
	void SetClearDepth(float depth);

	// Compute

	void DispatchCompute(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ);
	void DispatchComputeIndirect(intptr_t offset);

	// Draw

	void Draw(RenderPrimitiveMode mode, int32_t offset, int32_t vertexCount);
	void DrawIndexed(
		RenderPrimitiveMode mode,
		RenderIndexType indexType,
		int32_t indexCount,
		intptr_t indexOffset,
		int32_t baseVertex);
	void DrawIndexedInstanced(
		RenderPrimitiveMode mode,
		RenderIndexType indexType,
		int32_t indexCount,
		intptr_t indexOffset,
		int32_t instanceCount,
		int32_t baseVertex,
		uint32_t baseInstance);
	void DrawIndirect(RenderPrimitiveMode mode, intptr_t offset);
	void DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset);

	// Framebuffer

	void BindFramebuffer(render::FramebufferId framebuffer);

	// Sampler

	void BindSampler(uint32_t textureUnit, render::SamplerId sampler);

	// Shader

	void UseShaderProgram(render::ShaderId shader);

	// State

	void BlendingEnable();
	void BlendingDisable();
	void BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor);
	void SetBlendFunctionSeparate(
		uint32_t attachmentIndex,
		RenderBlendFactor srcFactorRgb,
		RenderBlendFactor dstFactorRgb,
		RenderBlendFactor srcFactorAlpha,
		RenderBlendFactor dstFactorAlpha);
	void SetBlendEquation(uint32_t attachmentIndex, RenderBlendEquation equation);

	void SetCullFace(RenderCullFace cullFace);

	void DepthTestEnable();
	void DepthTestDisable();
	void SetDepthTestFunction(RenderDepthCompareFunc function);
	void DepthWriteEnable();
	void DepthWriteDisable();

	void StencilTestDisable();

	void ScissorTestEnable();
	void ScissorTestDisable();
	void SetScissorRectangle(int32_t x, int32_t y, int32_t w, int32_t h);

	void SetViewport(int32_t x, int32_t y, int32_t w, int32_t h);

	// Texture

	void BindTextureToShader(
		int32_t uniformLocation,
		uint32_t textureUnit,
		render::TextureId texture);

	// Vertex arrays

	void BindVertexArray(render::VertexArrayId id);

	// Memory barrier

	void MemoryBarrier(const MemoryBarrierFlags& barrier);

private:
	void CopyCommand(const void* ptr, size_t size);
	uint32_t CopyCommandData(const void* ptr, size_t size);

	Allocator* allocator;
	CommandBuffer* buffer;
};

} // namespace render
} // namespace kokko
