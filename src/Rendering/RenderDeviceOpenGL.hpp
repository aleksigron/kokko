#pragma once

#include "Rendering/RenderDevice.hpp"

class RenderDeviceOpenGL : public RenderDevice
{
public:
	virtual void Clear(unsigned int mask) override;
	virtual void ClearColor(const RenderCommandData::ClearColorData* data) override;
	virtual void ClearDepth(float depth) override;

	virtual void BlendingEnable() override;
	virtual void BlendingDisable() override;
	virtual void BlendFunction(const RenderCommandData::BlendFunctionData* data) override;
	virtual void BlendFunction(unsigned int srcFactor, unsigned int dstFactor) override;

	virtual void DepthRange(const RenderCommandData::DepthRangeData* data) override;
	virtual void Viewport(const RenderCommandData::ViewportData* data) override;

	virtual void DepthTestEnable() override;
	virtual void DepthTestDisable() override;

	virtual void DepthTestFunction(unsigned int function) override;

	virtual void DepthWriteEnable() override;
	virtual void DepthWriteDisable() override;

	virtual void CullFaceEnable() override;
	virtual void CullFaceDisable() override;
	virtual void CullFaceFront() override;
	virtual void CullFaceBack() override;

	virtual void CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut) override;
	virtual void DestroyFramebuffers(unsigned int count, unsigned int* framebuffers) override;
	virtual void BindFramebuffer(const RenderCommandData::BindFramebufferData* data) override;
	virtual void BindFramebuffer(unsigned int target, unsigned int framebuffer) override;
	virtual void AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data) override;
	virtual void SetFramebufferDrawBuffers(unsigned int count, unsigned int* buffers) override;

	virtual void CreateTextures(unsigned int count, unsigned int* texturesOut) override;
	virtual void DestroyTextures(unsigned int count, unsigned int* textures) override;
	virtual void BindTexture(unsigned int target, unsigned int texture) override;
	virtual void SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data) override;
	virtual void SetTextureParameterInt(unsigned int target, unsigned int parameter, unsigned int value) override;
	virtual void SetActiveTextureUnit(unsigned int textureUnit) override;

	virtual unsigned int CreateShaderProgram() override;
	virtual void DestroyShaderProgram(unsigned int shaderProgram) override;
	virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) override;
	virtual void LinkShaderProgram(unsigned int shaderProgram) override;
	virtual void UseShaderProgram(unsigned int shaderProgram) override;
	virtual int GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter) override;
	virtual void GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut) override;

	virtual unsigned int CreateShaderStage(unsigned int shaderType) override;
	virtual void DestroyShaderStage(unsigned int shaderStage) override;
	virtual void SetShaderStageSource(unsigned int shaderStage, const char* source, int length) override;
	virtual void CompileShaderStage(unsigned int shaderStage) override;
	virtual int GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter) override;
	virtual void GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut) override;

	virtual int GetUniformLocation(unsigned int shaderProgram, const char* uniformName) override;
	virtual void SetUniformMat4x4f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec4f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec3f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformVec2f(int uniform, unsigned int count, const float* values) override;
	virtual void SetUniformFloat(int uniform, float value) override;
	virtual void SetUniformInt(int uniform, int value) override;

	virtual void BindVertexArray(unsigned int vertexArray) override;
	virtual void DrawVertexArray(unsigned int primitiveMode, int indexCount, unsigned int indexType) override;

	// Inherited via RenderDevice
};
