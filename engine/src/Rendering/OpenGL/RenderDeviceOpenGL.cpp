#include "Rendering/OpenGL/RenderDeviceOpenGL.hpp"

#include <cassert>

#include "System/IncludeOpenGL.hpp"

#include "Rendering/RenderDeviceEnumsOpenGL.hpp"

static void DebugMessageCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* userData)
{
	auto* data = static_cast<const RenderDeviceOpenGL::DebugMessageUserData*>(userData);

	RenderDevice::DebugMessage message{
		ConvertDebugSource(source),
		ConvertDebugType(type),
		id,
		ConvertDebugSeverity(severity),
		kokko::ConstStringView(msg, static_cast<unsigned int>(length))
	};

	if (data->callback)
		data->callback(message);
}

RenderDeviceOpenGL::RenderDeviceOpenGL() :
	debugUserData{ nullptr }
{
}

void RenderDeviceOpenGL::InitializeDefaults()
{
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
}

void RenderDeviceOpenGL::GetIntegerValue(RenderDeviceParameter parameter, int* valueOut)
{
	glGetIntegerv(ConvertDeviceParameter(parameter), valueOut);
}

void RenderDeviceOpenGL::SetDebugMessageCallback(DebugCallbackFn callback)
{
	debugUserData.callback = callback;
	glDebugMessageCallback(DebugMessageCallback, &debugUserData);
}

void RenderDeviceOpenGL::SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label)
{
	glObjectLabel(ConvertObjectType(type), object, static_cast<GLsizei>(label.len), label.str);
}

void RenderDeviceOpenGL::SetObjectPtrLabel(void* ptr, kokko::ConstStringView label)
{
	glObjectPtrLabel(ptr, static_cast<GLsizei>(label.len), label.str);
}

void RenderDeviceOpenGL::BeginDebugScope(uint32_t id, kokko::ConstStringView message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, message.len, message.str);
}

void RenderDeviceOpenGL::EndDebugScope()
{
	glPopDebugGroup();
}

void RenderDeviceOpenGL::CreateFramebuffers(unsigned int count, kokko::RenderFramebufferId* framebuffersOut)
{
	glCreateFramebuffers(count, &framebuffersOut[0].i);
}

void RenderDeviceOpenGL::DestroyFramebuffers(unsigned int count, const kokko::RenderFramebufferId* framebuffers)
{
	glDeleteFramebuffers(count, &framebuffers[0].i);
}

void RenderDeviceOpenGL::AttachFramebufferTexture(
	kokko::RenderFramebufferId framebuffer,
	RenderFramebufferAttachment attachment,
	kokko::RenderTextureId texture,
	int level)
{
	glNamedFramebufferTexture(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level);
}

void RenderDeviceOpenGL::AttachFramebufferTextureLayer(
	kokko::RenderFramebufferId framebuffer,
	RenderFramebufferAttachment attachment,
	kokko::RenderTextureId texture,
	int level,
	int layer)
{
	glNamedFramebufferTextureLayer(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level, layer);
}

void RenderDeviceOpenGL::SetFramebufferDrawBuffers(
	kokko::RenderFramebufferId framebuffer, unsigned int count, const RenderFramebufferAttachment* buffers)
{
	unsigned int attachments[16];

	for (unsigned int i = 0; i < count; ++i)
		attachments[i] = ConvertFramebufferAttachment(buffers[i]);

	glNamedFramebufferDrawBuffers(framebuffer.i, count, attachments);
}

void RenderDeviceOpenGL::ReadFramebufferPixels(int x, int y, int width, int height,
	RenderTextureBaseFormat format, RenderTextureDataType type, void* data)
{
	glReadPixels(x, y, width, height, ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

// TEXTURE

void RenderDeviceOpenGL::CreateTextures(
	RenderTextureTarget type,
	unsigned int count,
	kokko::RenderTextureId* texturesOut)
{
	glCreateTextures(ConvertTextureTarget(type), count, &texturesOut[0].i);
}

void RenderDeviceOpenGL::DestroyTextures(unsigned int count, const kokko::RenderTextureId* textures)
{
	glDeleteTextures(count, &textures[0].i);
}

void RenderDeviceOpenGL::SetTextureStorage2D(
	kokko::RenderTextureId texture,
	int levels,
	RenderTextureSizedFormat format,
	int width,
	int height)
{
	assert(levels > 0 && width > 0 && height > 0);
	glTextureStorage2D(texture.i, levels, ConvertTextureSizedFormat(format), width, height);
}

void RenderDeviceOpenGL::SetTextureSubImage2D(
	kokko::RenderTextureId texture,
	int level,
	int xOffset,
	int yOffset,
	int width,
	int height,
	RenderTextureBaseFormat format,
	RenderTextureDataType type,
	const void* data)
{
	glTextureSubImage2D(texture.i, level, xOffset, yOffset, width, height,
		ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

void RenderDeviceOpenGL::SetTextureSubImage3D(
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
	const void* data)
{
	glTextureSubImage3D(texture.i, level, xoffset, yoffset, zoffset, width, height, depth,
		ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

void RenderDeviceOpenGL::GenerateTextureMipmaps(kokko::RenderTextureId texture)
{
	glGenerateTextureMipmap(texture.i);
}

void RenderDeviceOpenGL::CreateSamplers(
	uint32_t count, const RenderSamplerParameters* params, kokko::RenderSamplerId* samplersOut)
{
	glGenSamplers(count, &samplersOut[0].i);

	for (uint32_t i = 0; i < count; ++i)
	{
		unsigned int sampler = samplersOut[i].i;
		const RenderSamplerParameters& data = params[i];
		glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, ConvertTextureFilterMode(data.minFilter));
		glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, ConvertTextureFilterMode(data.magFilter));
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, ConvertTextureWrapMode(data.wrapModeU));
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, ConvertTextureWrapMode(data.wrapModeV));
		glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, ConvertTextureWrapMode(data.wrapModeW));
		glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, ConvertTextureCompareMode(data.compareMode));
		glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, ConvertDepthCompareFunc(data.compareFunc));
	}
}

void RenderDeviceOpenGL::DestroySamplers(uint32_t count, const kokko::RenderSamplerId* samplers)
{
	glDeleteSamplers(count, &samplers[0].i);
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

int RenderDeviceOpenGL::GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter)
{
	int value = 0;
	glGetProgramiv(shaderProgram, parameter, &value);
	return value;
}

bool RenderDeviceOpenGL::GetShaderProgramLinkStatus(unsigned int shaderProgram)
{
	return GetShaderProgramParameterInt(shaderProgram, GL_LINK_STATUS) == GL_TRUE;
}

int RenderDeviceOpenGL::GetShaderProgramInfoLogLength(unsigned int shaderProgram)
{
	return GetShaderProgramParameterInt(shaderProgram, GL_INFO_LOG_LENGTH);
}

void RenderDeviceOpenGL::GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut)
{
	glGetProgramInfoLog(shaderProgram, maxLength, nullptr, logOut);
}

// SHADER STAGE

unsigned int RenderDeviceOpenGL::CreateShaderStage(RenderShaderStage stage)
{
	return glCreateShader(ConvertShaderStage(stage));
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

bool RenderDeviceOpenGL::GetShaderStageCompileStatus(unsigned int shaderStage)
{
	return GetShaderStageParameterInt(shaderStage, GL_COMPILE_STATUS) == GL_TRUE;
}

int RenderDeviceOpenGL::GetShaderStageInfoLogLength(unsigned int shaderStage)
{
	return GetShaderStageParameterInt(shaderStage, GL_INFO_LOG_LENGTH);
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

// VERTEX ARRAY

void RenderDeviceOpenGL::CreateVertexArrays(uint32_t count, kokko::RenderVertexArrayId* vertexArraysOut)
{
	glCreateVertexArrays(count, &vertexArraysOut[0].i);
}

void RenderDeviceOpenGL::DestroyVertexArrays(uint32_t count, const kokko::RenderVertexArrayId* vertexArrays)
{
	glDeleteVertexArrays(count, &vertexArrays[0].i);
}

void RenderDeviceOpenGL::EnableVertexAttribute(kokko::RenderVertexArrayId va, uint32_t attributeIndex)
{
	glEnableVertexArrayAttrib(va.i, attributeIndex);
}

void RenderDeviceOpenGL::SetVertexArrayIndexBuffer(kokko::RenderVertexArrayId va, kokko::RenderBufferId buffer)
{
	glVertexArrayElementBuffer(va.i, buffer.i);
}

void RenderDeviceOpenGL::SetVertexArrayVertexBuffer(
	kokko::RenderVertexArrayId va,
	uint32_t bindingIndex,
	kokko::RenderBufferId buffer,
	intptr_t offset,
	uint32_t stride)
{
	glVertexArrayVertexBuffer(va.i, bindingIndex, buffer.i, offset, stride);
}

void RenderDeviceOpenGL::SetVertexAttribFormat(
	kokko::RenderVertexArrayId va,
	uint32_t attributeIndex,
	uint32_t size,
	RenderVertexElemType elementType,
	uint32_t offset)
{
	glVertexArrayAttribFormat(va.i, attributeIndex, size, ConvertVertexElemType(elementType), GL_FALSE, offset);
}

void RenderDeviceOpenGL::SetVertexAttribBinding(
	kokko::RenderVertexArrayId va,
	uint32_t attributeIndex,
	uint32_t bindingIndex)
{
	glVertexArrayAttribBinding(va.i, attributeIndex, bindingIndex);
}

void RenderDeviceOpenGL::CreateBuffers(unsigned int count, kokko::RenderBufferId* buffersOut)
{
	glCreateBuffers(count, &buffersOut[0].i);
}

void RenderDeviceOpenGL::DestroyBuffers(unsigned int count, const kokko::RenderBufferId* buffers)
{
	glDeleteBuffers(count, &buffers[0].i);
}

void RenderDeviceOpenGL::SetBufferStorage(
	kokko::RenderBufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags)
{
	GLbitfield bits = 0;
	if (flags.dynamicStorage) bits |= GL_DYNAMIC_STORAGE_BIT;
	if (flags.mapReadAccess) bits |= GL_MAP_READ_BIT;
	if (flags.mapWriteAccess) bits |= GL_MAP_WRITE_BIT;
	if (flags.mapPersistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (flags.mapCoherent) bits |= GL_MAP_COHERENT_BIT;

	glNamedBufferStorage(buffer.i, size, data, bits);
}

void RenderDeviceOpenGL::SetBufferSubData(
	kokko::RenderBufferId buffer,
	unsigned int offset,
	unsigned int size,
	const void* data)
{
	glNamedBufferSubData(buffer.i, offset, size, data);
}

void* RenderDeviceOpenGL::MapBufferRange(
	kokko::RenderBufferId buffer,
	intptr_t offset,
	size_t length,
	BufferMapFlags flags)
{
	GLbitfield bits = 0;
	if (flags.readAccess) bits |= GL_MAP_READ_BIT;
	if (flags.writeAccess) bits |= GL_MAP_WRITE_BIT;
	if (flags.invalidateRange) bits |= GL_MAP_INVALIDATE_RANGE_BIT;
	if (flags.invalidateBuffer) bits |= GL_MAP_INVALIDATE_BUFFER_BIT;
	if (flags.flushExplicit) bits |= GL_MAP_FLUSH_EXPLICIT_BIT;
	if (flags.unsynchronized) bits |= GL_MAP_UNSYNCHRONIZED_BIT;
	if (flags.persistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (flags.coherent) bits |= GL_MAP_COHERENT_BIT;

	return glMapNamedBufferRange(buffer.i, offset, length, bits);
}

void RenderDeviceOpenGL::UnmapBuffer(kokko::RenderBufferId buffer)
{
	glUnmapNamedBuffer(buffer.i);
}
