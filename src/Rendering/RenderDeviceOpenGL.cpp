#include "Rendering/RenderDeviceOpenGL.hpp"

#include "System/IncludeOpenGL.hpp"

void RenderDeviceOpenGL::Clear(unsigned int mask)
{
	glClear(mask);
}

void RenderDeviceOpenGL::ClearColor(const RenderCommandData::ClearColorData* data)
{
	glClearColor(data->r, data->g, data->b, data->a);
}

void RenderDeviceOpenGL::ClearDepth(float depth)
{
	glClearDepth(depth);
}

void RenderDeviceOpenGL::BlendingEnable()
{
	glEnable(GL_BLEND);
}

void RenderDeviceOpenGL::BlendingDisable()
{
	glDisable(GL_BLEND);
}

void RenderDeviceOpenGL::BlendFunction(const RenderCommandData::BlendFunctionData* data)
{
	glBlendFunc(data->srcFactor, data->dstFactor);
}

void RenderDeviceOpenGL::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	glDepthRange(data->near, data->far);
}

void RenderDeviceOpenGL::Viewport(const RenderCommandData::ViewportData* data)
{
	glViewport(data->x, data->y, data->w, data->h);
}

void RenderDeviceOpenGL::DepthTestEnable()
{
	glEnable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestDisable()
{
	glDisable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestFunction(unsigned int function)
{
	glDepthFunc(function);
}

void RenderDeviceOpenGL::DepthWriteEnable()
{
	glDepthMask(GL_TRUE);
}

void RenderDeviceOpenGL::DepthWriteDisable()
{
	glDepthMask(GL_FALSE);
}

void RenderDeviceOpenGL::CullFaceEnable()
{
	glEnable(GL_CULL_FACE);
}

void RenderDeviceOpenGL::CullFaceDisable()
{
	glDisable(GL_CULL_FACE);
}

void RenderDeviceOpenGL::CullFaceFront()
{
	glCullFace(GL_FRONT);
}

void RenderDeviceOpenGL::CullFaceBack()
{
	glCullFace(GL_BACK);
}

void RenderDeviceOpenGL::CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut)
{
	glGenFramebuffers(count, framebuffersOut);
}

void RenderDeviceOpenGL::DestroyFramebuffers(unsigned int count, unsigned int* framebuffers)
{
	glDeleteFramebuffers(count, framebuffers);
}

void RenderDeviceOpenGL::BindFramebuffer(const RenderCommandData::BindFramebufferData* data)
{
	BindFramebuffer(data->target, data->framebuffer);
}

void RenderDeviceOpenGL::BindFramebuffer(unsigned int target, unsigned int framebuffer)
{
	glBindFramebuffer(target, framebuffer);
}

void RenderDeviceOpenGL::AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data)
{
	glFramebufferTexture2D(data->target, data->attachment, data->textureTarget, data->texture, data->mipLevel);
}

void RenderDeviceOpenGL::SetFramebufferDrawBuffers(unsigned int count, unsigned int* buffers)
{
	glDrawBuffers(count, buffers);
}

void RenderDeviceOpenGL::CreateTextures(unsigned int count, unsigned int* texturesOut)
{
	glGenTextures(count, texturesOut);
}

void RenderDeviceOpenGL::DestroyTextures(unsigned int count, unsigned int* textures)
{
	glDeleteTextures(count, textures);
}

void RenderDeviceOpenGL::BindTexture(unsigned int target, unsigned int texture)
{
	glBindTexture(target, texture);
}

void RenderDeviceOpenGL::SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data)
{
	glTexImage2D(data->target, data->mipLevel, data->internalFormat,
		data->width, data->height, 0, data->format, data->type, data->data);
}

void RenderDeviceOpenGL::SetTextureParameterInt(unsigned int target, unsigned int parameter, unsigned int value)
{
	glTexParameteri(target, parameter, value);
}

void RenderDeviceOpenGL::SetActiveTextureUnit(unsigned int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
}

void RenderDeviceOpenGL::UseShaderProgram(unsigned int shaderProgram)
{
	glUseProgram(shaderProgram);
}

int RenderDeviceOpenGL::GetUniformLocation(unsigned int shaderProgram, const char* uniformName)
{
	return glGetUniformLocation(shaderProgram, uniformName);
}

void RenderDeviceOpenGL::SetUniformMat4x4f(int uniform, unsigned int count, const float* values)
{
	glUniformMatrix4fv(uniform, count, GL_FALSE, values);
}

void RenderDeviceOpenGL::SetUniformVec4f(int uniform, unsigned int count, const float* values)
{
	glUniform4fv(uniform, count, values);
}

void RenderDeviceOpenGL::SetUniformVec3f(int uniform, unsigned int count, const float* values)
{
	glUniform3fv(uniform, count, values);
}

void RenderDeviceOpenGL::SetUniformVec2f(int uniform, unsigned int count, const float* values)
{
	glUniform2fv(uniform, count, values);
}

void RenderDeviceOpenGL::SetUniformFloat(int uniform, float value)
{
	glUniform1f(uniform, value);
}

void RenderDeviceOpenGL::SetUniformInt(int uniform, int value)
{
	glUniform1i(uniform, value);
}

void RenderDeviceOpenGL::BindVertexArray(unsigned int vertexArray)
{
	glBindVertexArray(vertexArray);
}

void RenderDeviceOpenGL::DrawVertexArray(unsigned int primitiveMode, int indexCount, unsigned int indexType)
{
	glDrawElements(primitiveMode, indexCount, indexType, nullptr);
}
