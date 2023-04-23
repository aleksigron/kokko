#pragma once

#include <cstdint>

#include "Core/StringView.hpp"

#include "Rendering/RenderDeviceDebugScope.hpp"
#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

class Allocator;

namespace kokko
{

// Dummy type to disallow implicit pointer casting
class NativeRenderDevice;

class CommandBuffer;

namespace render
{

class Device
{
public:
	struct DebugMessage
	{
		RenderDebugSource source;
		RenderDebugType type;
		unsigned int id;
		RenderDebugSeverity severity;
		ConstStringView message;
	};

	using DebugCallbackFn = void(*)(const DebugMessage&);

	static Device* Create(Allocator* allocator);

	virtual ~Device() {}

	virtual void InitializeDefaults() {}

	virtual NativeRenderDevice* GetNativeDevice() { return nullptr; }

	virtual CommandBuffer* CreateCommandBuffer(Allocator* allocator) { return nullptr; }

	virtual void GetIntegerValue(RenderDeviceParameter parameter, int* valueOut) = 0;

	virtual void SetDebugMessageCallback(DebugCallbackFn callback) = 0;
	virtual void SetObjectLabel(RenderObjectType type, unsigned int object, ConstStringView label) = 0;
	virtual void SetObjectPtrLabel(void* ptr, ConstStringView label) = 0;
	DeviceDebugScope CreateDebugScope(uint32_t id, ConstStringView message);
	virtual void BeginDebugScope(uint32_t id, ConstStringView message) = 0;
	virtual void EndDebugScope() = 0;

	virtual void CreateFramebuffers(unsigned int count, FramebufferId* framebuffersOut) = 0;
	virtual void DestroyFramebuffers(unsigned int count, const FramebufferId* framebuffers) = 0;
	virtual void AttachFramebufferTexture(
		FramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		TextureId texture,
		int level) = 0;
	virtual void AttachFramebufferTextureLayer(
		FramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		TextureId texture,
		int level,
		int layer) = 0;
	virtual void SetFramebufferDrawBuffers(
		FramebufferId framebuffer,
		unsigned int count,
		const RenderFramebufferAttachment* buffers) = 0;
	virtual void ReadFramebufferPixels(int x, int y, int width, int height,
		RenderTextureBaseFormat format, RenderTextureDataType type, void* data) = 0;

	virtual void CreateTextures(RenderTextureTarget type, unsigned int count, TextureId* texturesOut) = 0;
	virtual void DestroyTextures(unsigned int count, const TextureId* textures) = 0;
	virtual void SetTextureStorage2D(
		TextureId texture,
		int levels,
		RenderTextureSizedFormat format,
		int width,
		int height) = 0;
	virtual void SetTextureSubImage2D(
		TextureId texture,
		int level,
		int xOffset,
		int yOffset,
		int width,
		int height,
		RenderTextureBaseFormat format,
		RenderTextureDataType type,
		const void* data) = 0;
	virtual void SetTextureSubImage3D(
		TextureId texture,
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
	virtual void GenerateTextureMipmaps(TextureId texture) = 0;

	virtual void CreateSamplers(uint32_t count, const RenderSamplerParameters* params, SamplerId* samplersOut) = 0;
	virtual void DestroySamplers(uint32_t count, const SamplerId* samplers) = 0;

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

	virtual void CreateVertexArrays(uint32_t count, VertexArrayId* vertexArraysOut) = 0;
	virtual void DestroyVertexArrays(uint32_t count, const VertexArrayId* vertexArrays) = 0;
	virtual void EnableVertexAttribute(VertexArrayId va, uint32_t attributeIndex) = 0;
	virtual void SetVertexArrayIndexBuffer(VertexArrayId va, BufferId buffer) = 0;
	virtual void SetVertexArrayVertexBuffer(
		VertexArrayId va,
		uint32_t bindingIndex,
		BufferId buffer,
		intptr_t offset,
		uint32_t stride) = 0;
	virtual void SetVertexAttribFormat(
		VertexArrayId va,
		uint32_t attributeIndex,
		uint32_t size,
		RenderVertexElemType elementType,
		uint32_t offset) = 0;
	virtual void SetVertexAttribBinding(
		VertexArrayId va,
		uint32_t attributeIndex,
		uint32_t bindingIndex) = 0;

	virtual void CreateBuffers(unsigned int count, BufferId* buffersOut) = 0;
	virtual void DestroyBuffers(unsigned int count, const BufferId* buffers) = 0;
	virtual void SetBufferStorage(
		BufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags) = 0;
	virtual void SetBufferSubData(
		BufferId buffer, unsigned int offset, unsigned int size, const void* data) = 0;
	virtual void* MapBufferRange(
		BufferId buffer, intptr_t offset, size_t length, BufferMapFlags flags) = 0;
	virtual void UnmapBuffer(BufferId buffer) = 0;
};

} // namespace render
} // namespace kokko
