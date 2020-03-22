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
	BlendFunction(data->srcFactor, data->dstFactor);
}

void RenderDeviceOpenGL::BlendFunction(unsigned int srcFactor, unsigned int dstFactor)
{
	glBlendFunc(srcFactor, dstFactor);
}

void RenderDeviceOpenGL::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	glDepthRange(data->near, data->far);
}

void RenderDeviceOpenGL::Viewport(const RenderCommandData::ViewportData* data)
{
	glViewport(data->x, data->y, data->w, data->h);
}

// DEPTH TEST / WRITE

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

// CULL FACE

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

// FRAMEBUFFER

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

// TEXTURE

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

void RenderDeviceOpenGL::SetTextureImageCompressed2D(const RenderCommandData::SetTextureImageCompressed2D* data)
{
	glCompressedTexImage2D(data->target, data->mipLevel, data->internalFormat,
		data->width, data->height, 0, data->dataSize, data->data);
}

void RenderDeviceOpenGL::GenerateTextureMipmaps(unsigned int target)
{
	glGenerateMipmap(target);
}

void RenderDeviceOpenGL::SetTextureParameterInt(unsigned int target, unsigned int parameter, unsigned int value)
{
	glTexParameteri(target, parameter, value);
}

void RenderDeviceOpenGL::SetActiveTextureUnit(unsigned int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
}

// SHADER PROGRAM

unsigned int RenderDeviceOpenGL::CreateShaderProgram()
{
	return glCreateProgram();
}

void RenderDeviceOpenGL::DestroyShaderProgram(unsigned int shaderProgram)
{
	glDeleteProgram(shaderProgram);
}

void RenderDeviceOpenGL::AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage)
{
	glAttachShader(shaderProgram, shaderStage);
}

void RenderDeviceOpenGL::LinkShaderProgram(unsigned int shaderProgram)
{
	glLinkProgram(shaderProgram);
}

void RenderDeviceOpenGL::UseShaderProgram(unsigned int shaderProgram)
{
	glUseProgram(shaderProgram);
}

int RenderDeviceOpenGL::GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter)
{
	int value = 0;
	glGetProgramiv(shaderProgram, parameter, &value);
	return value;
}

void RenderDeviceOpenGL::GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut)
{
	glGetProgramInfoLog(shaderProgram, maxLength, nullptr, logOut);
}

// SHADER STAGE

unsigned int RenderDeviceOpenGL::CreateShaderStage(unsigned int shaderType)
{
	return glCreateShader(shaderType);
}

void RenderDeviceOpenGL::DestroyShaderStage(unsigned int shaderStage)
{
	glDeleteShader(shaderStage);
}

void RenderDeviceOpenGL::SetShaderStageSource(unsigned int shaderStage, const char* source, int length)
{
	glShaderSource(shaderStage, 1, &source, &length);
}

void RenderDeviceOpenGL::CompileShaderStage(unsigned int shaderStage)
{
	glCompileShader(shaderStage);
}

int RenderDeviceOpenGL::GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter)
{
	int value = 0;
	glGetShaderiv(shaderStage, parameter, &value);
	return value;
}

void RenderDeviceOpenGL::GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut)
{
	glGetShaderInfoLog(shaderStage, maxLength, nullptr, logOut);
}

// UNIFORM

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

// VERTEX ARRAY

void RenderDeviceOpenGL::CreateVertexArrays(unsigned int count, unsigned int* vertexArraysOut)
{
	glGenVertexArrays(count, vertexArraysOut);
}

void RenderDeviceOpenGL::DestroyVertexArrays(unsigned int count, unsigned int* vertexArrays)
{
	glDeleteVertexArrays(count, vertexArrays);
}

void RenderDeviceOpenGL::BindVertexArray(unsigned int vertexArray)
{
	glBindVertexArray(vertexArray);
}

void RenderDeviceOpenGL::DrawVertexArray(unsigned int primitiveMode, int indexCount, unsigned int indexType)
{
	glDrawElements(primitiveMode, indexCount, indexType, nullptr);
}

void RenderDeviceOpenGL::EnableVertexAttribute(unsigned int index)
{
	glEnableVertexAttribArray(index);
}

void RenderDeviceOpenGL::SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data)
{
	glVertexAttribPointer(data->attributeIndex, data->elementCount, data->elementType,
		GL_FALSE, data->stride, reinterpret_cast<void*>(data->offset));
}

void RenderDeviceOpenGL::CreateBuffers(unsigned int count, unsigned int* buffersOut)
{
	glGenBuffers(count, buffersOut);
}

void RenderDeviceOpenGL::DestroyBuffers(unsigned int count, unsigned int* buffers)
{
	glDeleteBuffers(count, buffers);
}

void RenderDeviceOpenGL::BindBuffer(unsigned int target, unsigned int buffer)
{
	glBindBuffer(target, buffer);
}

void RenderDeviceOpenGL::BindBufferBase(unsigned int target, unsigned int bindingPoint, unsigned int buffer)
{
	glBindBufferBase(target, bindingPoint, buffer);
}

void RenderDeviceOpenGL::SetBufferData(unsigned int target, unsigned int size, const void* data, unsigned int usage)
{
	glBufferData(target, size, data, usage);
}
