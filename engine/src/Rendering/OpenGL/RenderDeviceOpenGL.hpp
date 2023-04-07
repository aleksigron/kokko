#pragma once

#include "Rendering/RenderDevice.hpp"

class RenderDeviceOpenGL : public RenderDevice
{
public:
	struct DebugMessageUserData
	{
		DebugCallbackFn callback;
	};

private:
	DebugMessageUserData debugUserData;

public:
	RenderDeviceOpenGL();

	virtual void GetIntegerValue(RenderDeviceParameter parameter, int* valueOut) override;

	virtual void SetDebugMessageCallback(DebugCallbackFn callback) override;
	virtual void SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label) override;
	virtual void SetObjectPtrLabel(void* ptr, kokko::ConstStringView label) override;
	virtual void PushDebugGroup(unsigned int id, kokko::ConstStringView message) override;
	virtual void PopDebugGroup() override;

	//virtual void Clear(const RenderCommandData::ClearMask* data) override;
	//virtual void ClearColor(const RenderCommandData::ClearColorData* data) override;
	//virtual void ClearDepth(float depth) override;

	//virtual void BlendingEnable() override;
	//virtual void BlendingDisable() override;
	//virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) override;
	//virtual void BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor) override;

	//virtual void CubemapSeamlessEnable() override;
	//virtual void CubemapSeamlessDisable() override;
	//virtual void SetClipBehavior(RenderClipOriginMode origin, RenderClipDepthMode depth) override;
	//virtual void DepthRange(const RenderCommandData::DepthRangeData* data) override;
	//virtual void Viewport(const RenderCommandData::ViewportData* data) override;

	//virtual void ScissorTestEnable() override;
	//virtual void ScissorTestDisable() override;

	//virtual void DepthTestEnable() override;
	//virtual void DepthTestDisable() override;

	//virtual void DepthTestFunction(RenderDepthCompareFunc function) override;

	//virtual void DepthWriteEnable() override;
	//virtual void DepthWriteDisable() override;

	//virtual void CullFaceEnable() override;
	//virtual void CullFaceDisable() override;
	//virtual void CullFaceFront() override;
	//virtual void CullFaceBack() override;

	virtual void CreateFramebuffers(unsigned int count, kokko::RenderFramebufferId* framebuffersOut) override;
	virtual void DestroyFramebuffers(unsigned int count, const kokko::RenderFramebufferId* framebuffers) override;
	//virtual void BindFramebuffer(RenderFramebufferTarget target, kokko::RenderFramebufferId framebuffer) override;
	virtual void AttachFramebufferTexture(
		kokko::RenderFramebufferId framebuffer,
		RenderFramebufferAttachment attachment,
		kokko::RenderTextureId texture,
		int level) override;
	virtual void SetFramebufferDrawBuffers(unsigned int count, const RenderFramebufferAttachment* buffers) override;
	virtual void ReadFramebufferPixels(int x, int y, int width, int height,
		RenderTextureBaseFormat format, RenderTextureDataType type, void* data) override;

	virtual void CreateTextures(
		RenderTextureTarget type,
		unsigned int count,
		kokko::RenderTextureId* texturesOut) override;
	virtual void DestroyTextures(
		unsigned int count,
		const kokko::RenderTextureId* textures) override;
	virtual void SetTextureStorage2D(
		kokko::RenderTextureId texture,
		int levels,
		RenderTextureSizedFormat format,
		int width,
		int height) override;
	virtual void SetTextureSubImage2D(
		kokko::RenderTextureId texture,
		int level,
		int xOffset,
		int yOffset,
		int width,
		int height,
		RenderTextureBaseFormat format,
		RenderTextureDataType type,
		const void* data) override;
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
		const void* data) override;
	virtual void GenerateTextureMipmaps(kokko::RenderTextureId texture) override;

	void CreateSamplers(uint32_t count, const RenderSamplerParameters* params, kokko::RenderSamplerId* samplersOut) override;
	void DestroySamplers(uint32_t count, const kokko::RenderSamplerId* samplers) override;

	virtual unsigned int CreateShaderProgram() override;
	virtual void DestroyShaderProgram(unsigned int shaderProgram) override;
	virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) override;
	virtual void LinkShaderProgram(unsigned int shaderProgram) override;
	//virtual void UseShaderProgram(unsigned int shaderProgram) override;
	virtual bool GetShaderProgramLinkStatus(unsigned int shaderProgram) override;
	virtual int GetShaderProgramInfoLogLength(unsigned int shaderProgram) override;
	virtual int GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter) override;
	virtual void GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut) override;

	virtual unsigned int CreateShaderStage(RenderShaderStage stage) override;
	virtual void DestroyShaderStage(unsigned int shaderStage) override;
	virtual void SetShaderStageSource(unsigned int shaderStage, const char* source, int length) override;
	virtual void CompileShaderStage(unsigned int shaderStage) override;
	virtual int GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter) override;
	virtual bool GetShaderStageCompileStatus(unsigned int shaderStage) override;
	virtual int GetShaderStageInfoLogLength(unsigned int shaderStage) override;
	virtual void GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut) override;

	virtual int GetUniformLocation(unsigned int shaderProgram, const char* uniformName) override;
	//virtual void SetUniformMat4x4f(int uniform, unsigned int count, const float* values) override;
	//virtual void SetUniformVec4f(int uniform, unsigned int count, const float* values) override;
	//virtual void SetUniformVec3f(int uniform, unsigned int count, const float* values) override;
	//virtual void SetUniformVec2f(int uniform, unsigned int count, const float* values) override;
	//virtual void SetUniformFloat(int uniform, float value) override;
	//virtual void SetUniformInt(int uniform, int value) override;

	virtual void CreateVertexArrays(uint32_t count, kokko::RenderVertexArrayId* vertexArraysOut) override;
	virtual void DestroyVertexArrays(uint32_t count, const kokko::RenderVertexArrayId* vertexArrays) override;
	virtual void EnableVertexAttribute(kokko::RenderVertexArrayId va, uint32_t attributeIndex) override;
	virtual void SetVertexArrayIndexBuffer(kokko::RenderVertexArrayId va, kokko::RenderBufferId buffer) override;
	virtual void SetVertexArrayVertexBuffer(
		kokko::RenderVertexArrayId va,
		uint32_t bindingIndex,
		kokko::RenderBufferId buffer,
		intptr_t offset,
		uint32_t stride) override;
	virtual void SetVertexAttribFormat(
		kokko::RenderVertexArrayId va,
		uint32_t attributeIndex,
		uint32_t size,
		RenderVertexElemType elementType,
		uint32_t offset) override;
	virtual void SetVertexAttribBinding(
		kokko::RenderVertexArrayId va,
		uint32_t attributeIndex,
		uint32_t bindingIndex) override;

	virtual void CreateBuffers(unsigned int count, kokko::RenderBufferId* buffersOut) override;
	virtual void DestroyBuffers(unsigned int count, const kokko::RenderBufferId* buffers) override;
	virtual void SetBufferStorage(
		kokko::RenderBufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags) override;
	virtual void SetBufferSubData(
		kokko::RenderBufferId buffer, unsigned int offset, unsigned int size, const void* data) override;
	// virtual void* MapBuffer(RenderBufferTarget target, RenderBufferAccess access) override;
	virtual void* MapBufferRange(
		kokko::RenderBufferId buffer, intptr_t offset, size_t length, BufferMapFlags flags) override;
	virtual void UnmapBuffer(kokko::RenderBufferId buffer) override;

	//virtual void MemoryBarrier(const RenderCommandData::MemoryBarrier& barrier) override;
};
