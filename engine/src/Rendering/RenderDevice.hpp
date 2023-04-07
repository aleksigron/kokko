#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{
class CommandBuffer;
// Dummy type to disallow implicit pointer casting
class NativeRenderDevice;
}

class RenderDevice
{
public:
	struct DebugMessage
	{
		RenderDebugSource source;
		RenderDebugType type;
		unsigned int id;
		RenderDebugSeverity severity;
		kokko::ConstStringView message;
	};

	using DebugCallbackFn = void(*)(const DebugMessage&);

    static RenderDevice* Create(Allocator* allocator);

    virtual ~RenderDevice() {}

    virtual kokko::NativeRenderDevice* GetNativeDevice() { return nullptr; }

	virtual kokko::CommandBuffer* CreateCommandBuffer(Allocator* allocator) { return nullptr; }

	virtual void GetIntegerValue(RenderDeviceParameter parameter, int* valueOut) = 0;

	virtual void SetDebugMessageCallback(DebugCallbackFn callback) = 0;
	virtual void SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label) = 0;
	virtual void SetObjectPtrLabel(void* ptr, kokko::ConstStringView label) = 0;
	virtual void PushDebugGroup(unsigned int id, kokko::ConstStringView message) = 0;
	virtual void PopDebugGroup() = 0;

	//virtual void Clear(const RenderCommandData::ClearMask* data) = 0;
	//virtual void ClearColor(const RenderCommandData::ClearColorData* data) = 0;
	//virtual void ClearDepth(float depth) = 0;

	//virtual void BlendingEnable() = 0;
	//virtual void BlendingDisable() = 0;
	//virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) = 0;
	//virtual void BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor) = 0;

	//virtual void CubemapSeamlessEnable() = 0;
	//virtual void CubemapSeamlessDisable() = 0;
	//virtual void SetClipBehavior(RenderClipOriginMode origin, RenderClipDepthMode depth) = 0;
	//virtual void DepthRange(const RenderCommandData::DepthRangeData* data) = 0;
	//virtual void Viewport(const RenderCommandData::ViewportData* data) = 0;

	//virtual void ScissorTestEnable() = 0;
	//virtual void ScissorTestDisable() = 0;

	//virtual void DepthTestEnable() = 0;
	//virtual void DepthTestDisable() = 0;

	//virtual void DepthTestFunction(RenderDepthCompareFunc function) = 0;

	//virtual void DepthWriteEnable() = 0;
	//virtual void DepthWriteDisable() = 0;

	//virtual void CullFaceEnable() = 0;
	//virtual void CullFaceDisable() = 0;
	//virtual void CullFaceFront() = 0;
	//virtual void CullFaceBack() = 0;

	//virtual void FramebufferSrgbEnable() = 0;
	//virtual void FramebufferSrgbDisable() = 0;

	virtual void CreateFramebuffers(unsigned int count, kokko::RenderFramebufferId* framebuffersOut) = 0;
	virtual void DestroyFramebuffers(unsigned int count, const kokko::RenderFramebufferId* framebuffers) = 0;
	//virtual void BindFramebuffer(RenderFramebufferTarget target, kokko::RenderFramebufferId framebuffer) = 0;
	virtual void AttachFramebufferTexture(
		kokko::RenderFramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		kokko::RenderTextureId texture,
		int level) = 0;
	virtual void SetFramebufferDrawBuffers(unsigned int count, const RenderFramebufferAttachment* buffers) = 0;
	virtual void ReadFramebufferPixels(int x, int y, int width, int height,
		RenderTextureBaseFormat format, RenderTextureDataType type, void* data) = 0;

	virtual void CreateTextures(RenderTextureTarget type, unsigned int count, kokko::RenderTextureId* texturesOut) = 0;
	virtual void DestroyTextures(unsigned int count, const kokko::RenderTextureId* textures) = 0;
	virtual void SetTextureStorage2D(
		kokko::RenderTextureId texture,
		int levels,
		RenderTextureSizedFormat format,
		int width,
		int height) = 0;
	virtual void SetTextureSubImage2D(
		kokko::RenderTextureId texture,
		int level,
		int xOffset,
		int yOffset,
		int width,
		int height,
		RenderTextureBaseFormat format,
		RenderTextureDataType type,
		const void* data) = 0;
	virtual void SetTextureSubImage3D(
		kokko::RenderTextureId texture,
		int level,
		int xoffset,
		int yoffset,
		int zoffset,
		int width,
		int height,
		int depth,
		RenderTextureBaseFormat format,
		RenderTextureDataType type,
		const void* data) = 0;
	virtual void GenerateTextureMipmaps(kokko::RenderTextureId texture) = 0;

	virtual void CreateSamplers(uint32_t count, const RenderSamplerParameters* params, kokko::RenderSamplerId* samplersOut) = 0;
	virtual void DestroySamplers(uint32_t count, const kokko::RenderSamplerId* samplers) = 0;

	virtual unsigned int CreateShaderProgram() = 0;
	virtual void DestroyShaderProgram(unsigned int shaderProgram) = 0;
	virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) = 0;
	virtual void LinkShaderProgram(unsigned int shaderProgram) = 0;
	//virtual void UseShaderProgram(unsigned int shaderProgram) = 0;
	virtual int GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter) = 0;
	virtual bool GetShaderProgramLinkStatus(unsigned int shaderProgram) = 0;
	virtual int GetShaderProgramInfoLogLength(unsigned int shaderProgram) = 0;
	virtual void GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut) = 0;

	virtual unsigned int CreateShaderStage(RenderShaderStage stage) = 0;
	virtual void DestroyShaderStage(unsigned int shaderStage) = 0;
	virtual void SetShaderStageSource(unsigned int shaderStage, const char* source, int length) = 0;
	virtual void CompileShaderStage(unsigned int shaderStage) = 0;
	virtual int GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter) = 0;
	virtual bool GetShaderStageCompileStatus(unsigned int shaderStage) = 0;
	virtual int GetShaderStageInfoLogLength(unsigned int shaderStage) = 0;
	virtual void GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut) = 0;

	virtual int GetUniformLocation(unsigned int shaderProgram, const char* uniformName) = 0;
	//virtual void SetUniformMat4x4f(int uniform, unsigned int count, const float* values) = 0;
	//virtual void SetUniformVec4f(int uniform, unsigned int count, const float* values) = 0;
	//virtual void SetUniformVec3f(int uniform, unsigned int count, const float* values) = 0;
	//virtual void SetUniformVec2f(int uniform, unsigned int count, const float* values) = 0;
	//virtual void SetUniformFloat(int uniform, float value) = 0;
	//virtual void SetUniformInt(int uniform, int value) = 0;

	virtual void CreateVertexArrays(uint32_t count, kokko::RenderVertexArrayId* vertexArraysOut) = 0;
	virtual void DestroyVertexArrays(uint32_t count, const kokko::RenderVertexArrayId* vertexArrays) = 0;
	virtual void EnableVertexAttribute(kokko::RenderVertexArrayId va, uint32_t attributeIndex) = 0;
	virtual void SetVertexArrayIndexBuffer(kokko::RenderVertexArrayId va, kokko::RenderBufferId buffer) = 0;
	virtual void SetVertexArrayVertexBuffer(
		kokko::RenderVertexArrayId va,
		uint32_t bindingIndex,
		kokko::RenderBufferId buffer,
		intptr_t offset,
		uint32_t stride) = 0;
	virtual void SetVertexAttribFormat(
		kokko::RenderVertexArrayId va,
		uint32_t attributeIndex,
		uint32_t size,
		RenderVertexElemType elementType,
		uint32_t offset) = 0;
	virtual void SetVertexAttribBinding(
		kokko::RenderVertexArrayId va,
		uint32_t attributeIndex,
		uint32_t bindingIndex) = 0;

	virtual void CreateBuffers(unsigned int count, kokko::RenderBufferId* buffersOut) = 0;
	virtual void DestroyBuffers(unsigned int count, const kokko::RenderBufferId* buffers) = 0;
	virtual void SetBufferStorage(
		kokko::RenderBufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags) = 0;
	virtual void SetBufferSubData(
		kokko::RenderBufferId buffer, unsigned int offset, unsigned int size, const void* data) = 0;
	virtual void* MapBufferRange(
		kokko::RenderBufferId buffer, intptr_t offset, size_t length, BufferMapFlags flags) = 0;
	virtual void UnmapBuffer(kokko::RenderBufferId buffer) = 0;
};
