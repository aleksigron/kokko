#pragma once

#include "Rendering/RenderCommandData.hpp"
#include "Rendering/RenderDeviceEnums.hpp"

class RenderDevice
{
public:
	virtual ~RenderDevice() {}

	virtual void Clear(unsigned int mask) = 0;
	virtual void ClearColor(const RenderCommandData::ClearColorData* data) = 0;
	virtual void ClearDepth(float depth) = 0;

	virtual void BlendingEnable() = 0;
	virtual void BlendingDisable() = 0;
	virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) = 0;
	virtual void BlendFunction(unsigned int srcFactor, unsigned int dstFactor) = 0;

	virtual void DepthRange(const RenderCommandData::DepthRangeData* data) = 0;
	virtual void Viewport(const RenderCommandData::ViewportData* data) = 0;

	virtual void DepthTestEnable() = 0;
	virtual void DepthTestDisable() = 0;

	virtual void DepthTestFunction(unsigned int function) = 0;

	virtual void DepthWriteEnable() = 0;
	virtual void DepthWriteDisable() = 0;

	virtual void CullFaceEnable() = 0;
	virtual void CullFaceDisable() = 0;
	virtual void CullFaceFront() = 0;
	virtual void CullFaceBack() = 0;

	virtual void CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut) = 0;
	virtual void DestroyFramebuffers(unsigned int count, unsigned int* framebuffers) = 0;
	virtual void BindFramebuffer(const RenderCommandData::BindFramebufferData* data) = 0;
	virtual void BindFramebuffer(RenderFramebufferTarget target, unsigned int framebuffer) = 0;
	virtual void AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data) = 0;
	virtual void SetFramebufferDrawBuffers(unsigned int count, unsigned int* buffers) = 0;

	virtual void CreateTextures(unsigned int count, unsigned int* texturesOut) = 0;
	virtual void DestroyTextures(unsigned int count, unsigned int* textures) = 0;
	virtual void BindTexture(RenderTextureTarget target, unsigned int texture) = 0;
	virtual void SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data) = 0;
	virtual void SetTextureImageCompressed2D(const RenderCommandData::SetTextureImageCompressed2D* data) = 0;
	virtual void GenerateTextureMipmaps(RenderTextureTarget target) = 0;
	virtual void SetTextureParameterInt(RenderTextureTarget target, unsigned int parameter, unsigned int value) = 0;
	virtual void SetActiveTextureUnit(unsigned int textureUnit) = 0;

	virtual unsigned int CreateShaderProgram() = 0;
	virtual void DestroyShaderProgram(unsigned int shaderProgram) = 0;
	virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) = 0;
	virtual void LinkShaderProgram(unsigned int shaderProgram) = 0;
	virtual void UseShaderProgram(unsigned int shaderProgram) = 0;
	virtual int GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter) = 0;
	virtual void GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut) = 0;

	virtual unsigned int CreateShaderStage(unsigned int shaderType) = 0;
	virtual void DestroyShaderStage(unsigned int shaderStage) = 0;
	virtual void SetShaderStageSource(unsigned int shaderStage, const char* source, int length) = 0;
	virtual void CompileShaderStage(unsigned int shaderStage) = 0;
	virtual int GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter) = 0;
	virtual void GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut) = 0;

	virtual int GetUniformLocation(unsigned int shaderProgram, const char* uniformName) = 0;
	virtual void SetUniformMat4x4f(int uniform, unsigned int count, const float* values) = 0;
	virtual void SetUniformVec4f(int uniform, unsigned int count, const float* values) = 0;
	virtual void SetUniformVec3f(int uniform, unsigned int count, const float* values) = 0;
	virtual void SetUniformVec2f(int uniform, unsigned int count, const float* values) = 0;
	virtual void SetUniformFloat(int uniform, float value) = 0;
	virtual void SetUniformInt(int uniform, int value) = 0;

	virtual void CreateVertexArrays(unsigned int count, unsigned int* vertexArraysOut) = 0;
	virtual void DestroyVertexArrays(unsigned int count, unsigned int* vertexArrays) = 0;
	virtual void BindVertexArray(unsigned int vertexArray) = 0;
	virtual void DrawIndexed(unsigned int primitiveMode, int indexCount, unsigned int indexType) = 0;
	virtual void Draw(unsigned int primitiveMode, int offset, int vertexCount) = 0;
	virtual void EnableVertexAttribute(unsigned int index) = 0;
	virtual void SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data) = 0;

	virtual void CreateBuffers(unsigned int count, unsigned int* buffersOut) = 0;
	virtual void DestroyBuffers(unsigned int count, unsigned int* buffers) = 0;
	virtual void BindBuffer(RenderBufferTarget target, unsigned int buffer) = 0;
	virtual void BindBufferBase(RenderBufferTarget target, unsigned int bindingPoint, unsigned int buffer) = 0;
	virtual void SetBufferData(RenderBufferTarget target, unsigned int size, const void* data, RenderBufferUsage usage) = 0;
	virtual void SetBufferSubData(RenderBufferTarget target, unsigned int offset, unsigned int size, const void* data) = 0;
};
