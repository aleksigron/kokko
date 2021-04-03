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
	virtual void SetObjectLabel(RenderObjectType type, unsigned int object, StringRef label) override;
	virtual void SetObjectPtrLabel(void* ptr, StringRef label) override;
	virtual void PushDebugGroup(unsigned int id, StringRef message) override;
	virtual void PopDebugGroup() override;

	virtual void Clear(const RenderCommandData::ClearMask* data) override;
	virtual void ClearColor(const RenderCommandData::ClearColorData* data) override;
	virtual void ClearDepth(float depth) override;

	virtual void BlendingEnable() override;
	virtual void BlendingDisable() override;
	virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) override;
	virtual void BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor) override;

	virtual void CubemapSeamlessEnable() override;
	virtual void CubemapSeamlessDisable() override;
	virtual void SetClipBehavior(RenderClipOriginMode origin, RenderClipDepthMode depth) override;
	virtual void DepthRange(const RenderCommandData::DepthRangeData* data) override;
	virtual void Viewport(const RenderCommandData::ViewportData* data) override;

	virtual void ScissorTestEnable() override;
	virtual void ScissorTestDisable() override;

	virtual void DepthTestEnable() override;
	virtual void DepthTestDisable() override;

	virtual void DepthTestFunction(RenderDepthCompareFunc function) override;

	virtual void DepthWriteEnable() override;
	virtual void DepthWriteDisable() override;

	virtual void CullFaceEnable() override;
	virtual void CullFaceDisable() override;
	virtual void CullFaceFront() override;
	virtual void CullFaceBack() override;

	virtual void FramebufferSrgbEnable() override;
	virtual void FramebufferSrgbDisable() override;

	virtual void CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut) override;
	virtual void DestroyFramebuffers(unsigned int count, unsigned int* framebuffers) override;
	virtual void BindFramebuffer(const RenderCommandData::BindFramebufferData* data) override;
	virtual void BindFramebuffer(RenderFramebufferTarget target, unsigned int framebuffer) override;
	virtual void AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data) override;
	virtual void SetFramebufferDrawBuffers(unsigned int count, RenderFramebufferAttachment* buffers) override;

	virtual void CreateTextures(unsigned int count, unsigned int* texturesOut) override;
	virtual void DestroyTextures(unsigned int count, unsigned int* textures) override;
	virtual void BindTexture(RenderTextureTarget target, unsigned int texture) override;
	virtual void SetTextureStorage2D(const RenderCommandData::SetTextureStorage2D* data) override;
	virtual void SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data) override;
	virtual void SetTextureSubImage2D(const RenderCommandData::SetTextureSubImage2D* data) override;
	virtual void SetTextureImageCompressed2D(const RenderCommandData::SetTextureImageCompressed2D* data) override;
	virtual void GenerateTextureMipmaps(RenderTextureTarget target) override;
	virtual void SetActiveTextureUnit(unsigned int textureUnit) override;

	virtual void SetTextureParameterInt(RenderTextureTarget target, RenderTextureParameter parameter, unsigned int value) override;
	virtual void SetTextureMinFilter(RenderTextureTarget target, RenderTextureFilterMode mode) override;
	virtual void SetTextureMagFilter(RenderTextureTarget target, RenderTextureFilterMode mode) override;
	virtual void SetTextureWrapModeU(RenderTextureTarget target, RenderTextureWrapMode mode) override;
	virtual void SetTextureWrapModeV(RenderTextureTarget target, RenderTextureWrapMode mode) override;
	virtual void SetTextureWrapModeW(RenderTextureTarget target, RenderTextureWrapMode mode) override;
	virtual void SetTextureCompareMode(RenderTextureTarget target, RenderTextureCompareMode mode) override;
	virtual void SetTextureCompareFunc(RenderTextureTarget target, RenderDepthCompareFunc func) override;

	virtual void CreateSamplers(unsigned int count, unsigned int* samplersOut) override;
	virtual void DestroySamplers(unsigned int count, unsigned int* samplers) override;
	virtual void BindSampler(unsigned int textureUnit, unsigned int sampler) override;
	virtual void SetSamplerParameters(const RenderCommandData::SetSamplerParameters* data) override;

	virtual unsigned int CreateShaderProgram() override;
	virtual void DestroyShaderProgram(unsigned int shaderProgram) override;
	virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) override;
	virtual void LinkShaderProgram(unsigned int shaderProgram) override;
	virtual void UseShaderProgram(unsigned int shaderProgram) override;
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
	virtual void SetUniformMat4x4f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec4f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec3f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec2f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformFloat(int uniform, float value) override;
	virtual void SetUniformInt(int uniform, int value) override;

	virtual void CreateVertexArrays(unsigned int count, unsigned int* vertexArraysOut) override;
	virtual void DestroyVertexArrays(unsigned int count, unsigned int* vertexArrays) override;
	virtual void BindVertexArray(unsigned int vertexArrayId) override;
	virtual void EnableVertexAttribute(unsigned int index) override;
	virtual void SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data) override;

	virtual void Draw(RenderPrimitiveMode mode, int offset, int vertexCount) override;
	virtual void DrawIndexed(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType) override;
	virtual void DrawInstanced(RenderPrimitiveMode mode, int offset, int vertexCount, int instanceCount) override;
	virtual void DrawIndexedInstanced(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType, int instanceCount) override;
	virtual void DrawIndirect(RenderPrimitiveMode mode, intptr_t offset) override;
	virtual void DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset) override;

	virtual void CreateBuffers(unsigned int count, unsigned int* buffersOut) override;
	virtual void DestroyBuffers(unsigned int count, unsigned int* buffers) override;
	virtual void BindBuffer(RenderBufferTarget target, unsigned int buffer) override;
	virtual void BindBufferBase(RenderBufferTarget target, unsigned int bindingPoint, unsigned int buffer) override;
	virtual void BindBufferRange(const RenderCommandData::BindBufferRange* data) override;
	virtual void SetBufferStorage(const RenderCommandData::SetBufferStorage* data) override;
	virtual void SetBufferData(RenderBufferTarget target, unsigned int size, const void* data, RenderBufferUsage usage) override;
	virtual void SetBufferSubData(RenderBufferTarget target, unsigned int offset, unsigned int size, const void* data) override;
	virtual void* MapBuffer(RenderBufferTarget target, RenderBufferAccess access) override;
	virtual void* MapBufferRange(const RenderCommandData::MapBufferRange* data) override;
	virtual void UnmapBuffer(RenderBufferTarget target) override;

	virtual void DispatchCompute(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ) override;
	virtual void DispatchComputeIndirect(intptr_t offset) override;

	virtual void MemoryBarrier(const RenderCommandData::MemoryBarrier& barrier) override;
};
