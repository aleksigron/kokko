#include "Rendering/RenderDeviceOpenGL.hpp"

#include <cassert>

#include "System/IncludeOpenGL.hpp"

#include "Rendering/RenderDeviceEnumsOpenGL.hpp"

namespace kokko
{
namespace render
{

namespace
{
static void DebugMessageCallback(
	GLenum source, GLenum type, GLuint id, GLenum severity,
	GLsizei length, const GLchar* msg, const void* userData)
{
	auto* data = static_cast<const DeviceOpenGL::DebugMessageUserData*>(userData);

	Device::DebugMessage message{
		ConvertDebugSource(source),
		ConvertDebugType(type),
		id,
		ConvertDebugSeverity(severity),
		ConstStringView(msg, static_cast<unsigned int>(length))
	};

	if (data->callback)
		data->callback(message);
}
}

DeviceOpenGL::DeviceOpenGL() :
	debugUserData{ nullptr }
{
}

void DeviceOpenGL::InitializeDefaults()
{
	glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
}

void DeviceOpenGL::GetIntegerValue(RenderDeviceParameter parameter, int* valueOut)
{
	glGetIntegerv(ConvertDeviceParameter(parameter), valueOut);
}

void DeviceOpenGL::SetDebugMessageCallback(DebugCallbackFn callback)
{
	debugUserData.callback = callback;
	glDebugMessageCallback(DebugMessageCallback, &debugUserData);
}

void DeviceOpenGL::SetObjectLabel(RenderObjectType type, unsigned int object, ConstStringView label)
{
	glObjectLabel(ConvertObjectType(type), object, static_cast<GLsizei>(label.len), label.str);
}

void DeviceOpenGL::SetObjectPtrLabel(void* ptr, ConstStringView label)
{
	glObjectPtrLabel(ptr, static_cast<GLsizei>(label.len), label.str);
}

void DeviceOpenGL::BeginDebugScope(uint32_t id, ConstStringView message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, message.len, message.str);
}

void DeviceOpenGL::EndDebugScope()
{
	glPopDebugGroup();
}

void DeviceOpenGL::CreateFramebuffers(unsigned int count, FramebufferId* framebuffersOut)
{
	glCreateFramebuffers(count, &framebuffersOut[0].i);
}

void DeviceOpenGL::DestroyFramebuffers(unsigned int count, const FramebufferId* framebuffers)
{
	glDeleteFramebuffers(count, &framebuffers[0].i);
}

void DeviceOpenGL::AttachFramebufferTexture(
	FramebufferId framebuffer,
	RenderFramebufferAttachment attachment,
	TextureId texture,
	int level)
{
	glNamedFramebufferTexture(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level);
}

void DeviceOpenGL::AttachFramebufferTextureLayer(
	FramebufferId framebuffer,
	RenderFramebufferAttachment attachment,
	TextureId texture,
	int level,
	int layer)
{
	glNamedFramebufferTextureLayer(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level, layer);
}

void DeviceOpenGL::SetFramebufferDrawBuffers(
	FramebufferId framebuffer, unsigned int count, const RenderFramebufferAttachment* buffers)
{
	unsigned int attachments[16];

	for (unsigned int i = 0; i < count; ++i)
		attachments[i] = ConvertFramebufferAttachment(buffers[i]);

	glNamedFramebufferDrawBuffers(framebuffer.i, count, attachments);
}

void DeviceOpenGL::ReadFramebufferPixels(int x, int y, int width, int height,
	RenderTextureBaseFormat format, RenderTextureDataType type, void* data)
{
	glReadPixels(x, y, width, height, ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

// TEXTURE

void DeviceOpenGL::CreateTextures(
	RenderTextureTarget type,
	unsigned int count,
	TextureId* texturesOut)
{
	glCreateTextures(ConvertTextureTarget(type), count, &texturesOut[0].i);
}

void DeviceOpenGL::DestroyTextures(unsigned int count, const TextureId* textures)
{
	glDeleteTextures(count, &textures[0].i);
}

void DeviceOpenGL::SetTextureStorage2D(
	TextureId texture,
	int levels,
	RenderTextureSizedFormat format,
	int width,
	int height)
{
	assert(levels > 0 && width > 0 && height > 0);
	glTextureStorage2D(texture.i, levels, ConvertTextureSizedFormat(format), width, height);
}

void DeviceOpenGL::SetTextureSubImage2D(
	TextureId texture,
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

void DeviceOpenGL::SetTextureSubImage3D(
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
	const void* data)
{
	glTextureSubImage3D(texture.i, level, xoffset, yoffset, zoffset, width, height, depth,
		ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

void DeviceOpenGL::GenerateTextureMipmaps(TextureId texture)
{
	glGenerateTextureMipmap(texture.i);
}

void DeviceOpenGL::CreateSamplers(
	uint32_t count, const RenderSamplerParameters* params, SamplerId* samplersOut)
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

void DeviceOpenGL::DestroySamplers(uint32_t count, const SamplerId* samplers)
{
	glDeleteSamplers(count, &samplers[0].i);
}

// SHADER PROGRAM

unsigned int DeviceOpenGL::CreateShaderProgram()
{
	return glCreateProgram();
}

void DeviceOpenGL::DestroyShaderProgram(unsigned int shaderProgram)
{
	glDeleteProgram(shaderProgram);
}

void DeviceOpenGL::AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage)
{
	glAttachShader(shaderProgram, shaderStage);
}

void DeviceOpenGL::LinkShaderProgram(unsigned int shaderProgram)
{
	glLinkProgram(shaderProgram);
}

int DeviceOpenGL::GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter)
{
	int value = 0;
	glGetProgramiv(shaderProgram, parameter, &value);
	return value;
}

bool DeviceOpenGL::GetShaderProgramLinkStatus(unsigned int shaderProgram)
{
	return GetShaderProgramParameterInt(shaderProgram, GL_LINK_STATUS) == GL_TRUE;
}

int DeviceOpenGL::GetShaderProgramInfoLogLength(unsigned int shaderProgram)
{
	return GetShaderProgramParameterInt(shaderProgram, GL_INFO_LOG_LENGTH);
}

void DeviceOpenGL::GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut)
{
	glGetProgramInfoLog(shaderProgram, maxLength, nullptr, logOut);
}

// SHADER STAGE

unsigned int DeviceOpenGL::CreateShaderStage(RenderShaderStage stage)
{
	return glCreateShader(ConvertShaderStage(stage));
}

void DeviceOpenGL::DestroyShaderStage(unsigned int shaderStage)
{
	glDeleteShader(shaderStage);
}

void DeviceOpenGL::SetShaderStageSource(unsigned int shaderStage, const char* source, int length)
{
	glShaderSource(shaderStage, 1, &source, &length);
}

void DeviceOpenGL::CompileShaderStage(unsigned int shaderStage)
{
	glCompileShader(shaderStage);
}

int DeviceOpenGL::GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter)
{
	int value = 0;
	glGetShaderiv(shaderStage, parameter, &value);
	return value;
}

bool DeviceOpenGL::GetShaderStageCompileStatus(unsigned int shaderStage)
{
	return GetShaderStageParameterInt(shaderStage, GL_COMPILE_STATUS) == GL_TRUE;
}

int DeviceOpenGL::GetShaderStageInfoLogLength(unsigned int shaderStage)
{
	return GetShaderStageParameterInt(shaderStage, GL_INFO_LOG_LENGTH);
}

void DeviceOpenGL::GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut)
{
	glGetShaderInfoLog(shaderStage, maxLength, nullptr, logOut);
}

// UNIFORM

int DeviceOpenGL::GetUniformLocation(unsigned int shaderProgram, const char* uniformName)
{
	return glGetUniformLocation(shaderProgram, uniformName);
}

// VERTEX ARRAY

void DeviceOpenGL::CreateVertexArrays(uint32_t count, VertexArrayId* vertexArraysOut)
{
	glCreateVertexArrays(count, &vertexArraysOut[0].i);
}

void DeviceOpenGL::DestroyVertexArrays(uint32_t count, const VertexArrayId* vertexArrays)
{
	glDeleteVertexArrays(count, &vertexArrays[0].i);
}

void DeviceOpenGL::EnableVertexAttribute(VertexArrayId va, uint32_t attributeIndex)
{
	glEnableVertexArrayAttrib(va.i, attributeIndex);
}

void DeviceOpenGL::SetVertexArrayIndexBuffer(VertexArrayId va, BufferId buffer)
{
	glVertexArrayElementBuffer(va.i, buffer.i);
}

void DeviceOpenGL::SetVertexArrayVertexBuffer(
	VertexArrayId va,
	uint32_t bindingIndex,
	BufferId buffer,
	intptr_t offset,
	uint32_t stride)
{
	glVertexArrayVertexBuffer(va.i, bindingIndex, buffer.i, offset, stride);
}

void DeviceOpenGL::SetVertexAttribFormat(
	VertexArrayId va,
	uint32_t attributeIndex,
	uint32_t size,
	RenderVertexElemType elementType,
	uint32_t offset)
{
	glVertexArrayAttribFormat(va.i, attributeIndex, size, ConvertVertexElemType(elementType), GL_FALSE, offset);
}

void DeviceOpenGL::SetVertexAttribBinding(
	VertexArrayId va,
	uint32_t attributeIndex,
	uint32_t bindingIndex)
{
	glVertexArrayAttribBinding(va.i, attributeIndex, bindingIndex);
}

void DeviceOpenGL::CreateBuffers(unsigned int count, BufferId* buffersOut)
{
	glCreateBuffers(count, &buffersOut[0].i);
}

void DeviceOpenGL::DestroyBuffers(unsigned int count, const BufferId* buffers)
{
	glDeleteBuffers(count, &buffers[0].i);
}

void DeviceOpenGL::SetBufferStorage(
	BufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags)
{
	GLbitfield bits = 0;
	if (flags.dynamicStorage) bits |= GL_DYNAMIC_STORAGE_BIT;
	if (flags.mapReadAccess) bits |= GL_MAP_READ_BIT;
	if (flags.mapWriteAccess) bits |= GL_MAP_WRITE_BIT;
	if (flags.mapPersistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (flags.mapCoherent) bits |= GL_MAP_COHERENT_BIT;

	glNamedBufferStorage(buffer.i, size, data, bits);
}

void DeviceOpenGL::SetBufferSubData(
	BufferId buffer,
	unsigned int offset,
	unsigned int size,
	const void* data)
{
	glNamedBufferSubData(buffer.i, offset, size, data);
}

void* DeviceOpenGL::MapBufferRange(
	BufferId buffer,
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

void DeviceOpenGL::UnmapBuffer(BufferId buffer)
{
	glUnmapNamedBuffer(buffer.i);
}

} // namespace render
} // namespace kokko
