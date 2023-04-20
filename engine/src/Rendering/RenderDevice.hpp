#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

#include "Rendering/RenderDeviceDebugScope.hpp"
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
private:
	friend class kokko::RenderDeviceDebugScope;

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

	virtual void InitializeDefaults() {}

    virtual kokko::NativeRenderDevice* GetNativeDevice() { return nullptr; }

	virtual kokko::CommandBuffer* CreateCommandBuffer(Allocator* allocator) { return nullptr; }

	virtual void GetIntegerValue(RenderDeviceParameter parameter, int* valueOut) = 0;

	virtual void SetDebugMessageCallback(DebugCallbackFn callback) = 0;
	virtual void SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label) = 0;
	virtual void SetObjectPtrLabel(void* ptr, kokko::ConstStringView label) = 0;
	kokko::RenderDeviceDebugScope CreateDebugScope(uint32_t id, kokko::ConstStringView message);
	virtual void BeginDebugScope(uint32_t id, kokko::ConstStringView message) = 0;
	virtual void EndDebugScope() = 0;

	virtual void CreateFramebuffers(unsigned int count, kokko::RenderFramebufferId* framebuffersOut) = 0;
	virtual void DestroyFramebuffers(unsigned int count, const kokko::RenderFramebufferId* framebuffers) = 0;
	virtual void AttachFramebufferTexture(
		kokko::RenderFramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		kokko::RenderTextureId texture,
		int level) = 0;
	virtual void AttachFramebufferTextureLayer(
		kokko::RenderFramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		kokko::RenderTextureId texture,
		int level,
		int layer) = 0;
	virtual void SetFramebufferDrawBuffers(
		kokko::RenderFramebufferId framebuffer,
		unsigned int count,
		const RenderFramebufferAttachment* buffers) = 0;
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
