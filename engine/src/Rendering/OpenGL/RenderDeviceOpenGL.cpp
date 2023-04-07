#include "Rendering/OpenGL/RenderDeviceOpenGL.hpp"

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

void RenderDeviceOpenGL::PushDebugGroup(unsigned int id, kokko::ConstStringView message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(message.len), message.str);
}

void RenderDeviceOpenGL::PopDebugGroup()
{
	glPopDebugGroup();
}

/*
void RenderDeviceOpenGL::Clear(const RenderCommandData::ClearMask* data)
{
	unsigned int mask = 0;

	if (data->color) mask |= GL_COLOR_BUFFER_BIT;
	if (data->depth) mask |= GL_DEPTH_BUFFER_BIT;
	if (data->stencil) mask |= GL_STENCIL_BUFFER_BIT;

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

void RenderDeviceOpenGL::BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor)
{
	glBlendFunc(ConvertBlendFactor(srcFactor), ConvertBlendFactor(dstFactor));
}

void RenderDeviceOpenGL::CubemapSeamlessEnable()
{
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void RenderDeviceOpenGL::CubemapSeamlessDisable()
{
	glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void RenderDeviceOpenGL::SetClipBehavior(RenderClipOriginMode origin, RenderClipDepthMode depth)
{
	glClipControl(ConvertClipOriginMode(origin), ConvertClipDepthMode(depth));
}

void RenderDeviceOpenGL::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	glDepthRange(data->near, data->far);
}

void RenderDeviceOpenGL::Viewport(const RenderCommandData::ViewportData* data)
{
	glViewport(data->x, data->y, data->w, data->h);
}

void RenderDeviceOpenGL::ScissorTestEnable()
{
	glEnable(GL_SCISSOR_TEST);
}

void RenderDeviceOpenGL::ScissorTestDisable()
{
	glDisable(GL_SCISSOR_TEST);
}

void RenderDeviceOpenGL::DepthTestEnable()
{
	glEnable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestDisable()
{
	glDisable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestFunction(RenderDepthCompareFunc function)
{
	glDepthFunc(ConvertDepthCompareFunc(function));
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

void RenderDeviceOpenGL::FramebufferSrgbEnable()
{
	glEnable(GL_FRAMEBUFFER_SRGB);
}

void RenderDeviceOpenGL::FramebufferSrgbDisable()
{
	glDisable(GL_FRAMEBUFFER_SRGB);
}
*/
void RenderDeviceOpenGL::CreateFramebuffers(unsigned int count, kokko::RenderFramebufferId* framebuffersOut)
{
	glGenFramebuffers(count, &framebuffersOut[0].i);
}

void RenderDeviceOpenGL::DestroyFramebuffers(unsigned int count, const kokko::RenderFramebufferId* framebuffers)
{
	glDeleteFramebuffers(count, &framebuffers[0].i);
}

/*
void RenderDeviceOpenGL::BindFramebuffer(RenderFramebufferTarget target, kokko::RenderFramebufferId framebuffer)
{
	glBindFramebuffer(ConvertFramebufferTarget(target), framebuffer);
}
*/

void RenderDeviceOpenGL::AttachFramebufferTexture(
	kokko::RenderFramebufferId framebuffer,
	RenderFramebufferAttachment attachment,
	kokko::RenderTextureId texture,
	int level)
{
	glNamedFramebufferTexture(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level);
}

void RenderDeviceOpenGL::SetFramebufferDrawBuffers(unsigned int count, const RenderFramebufferAttachment* buffers)
{
	unsigned int attachments[16];

	for (unsigned int i = 0; i < count; ++i)
		attachments[i] = ConvertFramebufferAttachment(buffers[i]);

	glDrawBuffers(count, attachments);
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
	glCreateTextures(ConvertTextureTarget(type), count, &texturesOut	[0].i);
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

/*
void RenderDeviceOpenGL::SetActiveTextureUnit(unsigned int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
}

void RenderDeviceOpenGL::SetTextureParameterInt(RenderTextureTarget target, RenderTextureParameter parameter, unsigned int value)
{
	glTexParameteri(ConvertTextureTarget(target), ConvertTextureParameter(parameter), value);
}

void RenderDeviceOpenGL::SetTextureMinFilter(RenderTextureTarget target, RenderTextureFilterMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::MinificationFilter, ConvertTextureFilterMode(mode));
}

void RenderDeviceOpenGL::SetTextureMagFilter(RenderTextureTarget target, RenderTextureFilterMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::MagnificationFilter, ConvertTextureFilterMode(mode));
}

void RenderDeviceOpenGL::SetTextureWrapModeU(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::WrapModeU, ConvertTextureWrapMode(mode));
}

void RenderDeviceOpenGL::SetTextureWrapModeV(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::WrapModeV, ConvertTextureWrapMode(mode));
}

void RenderDeviceOpenGL::SetTextureWrapModeW(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::WrapModeW, ConvertTextureWrapMode(mode));
}

void RenderDeviceOpenGL::SetTextureCompareMode(RenderTextureTarget target, RenderTextureCompareMode mode)
{
	SetTextureParameterInt(target, RenderTextureParameter::CompareMode, ConvertTextureCompareMode(mode));
}

void RenderDeviceOpenGL::SetTextureCompareFunc(RenderTextureTarget target, RenderDepthCompareFunc func)
{
	SetTextureParameterInt(target, RenderTextureParameter::CompareFunc, ConvertDepthCompareFunc(func));
}
*/
void RenderDeviceOpenGL::CreateSamplers(uint32_t count, const RenderSamplerParameters* params, kokko::RenderSamplerId* samplersOut)
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
/*
void RenderDeviceOpenGL::BindSampler(unsigned int textureUnit, unsigned int sampler)
{
	glBindSampler(textureUnit, sampler);
}

void RenderDeviceOpenGL::SetSamplerParameters(const RenderCommandData::SetSamplerParameters* data)
{
	glSamplerParameteri(data->sampler, GL_TEXTURE_MIN_FILTER, ConvertTextureFilterMode(data->minFilter));
	glSamplerParameteri(data->sampler, GL_TEXTURE_MAG_FILTER, ConvertTextureFilterMode(data->magFilter));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_S, ConvertTextureWrapMode(data->wrapModeU));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_T, ConvertTextureWrapMode(data->wrapModeV));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_R, ConvertTextureWrapMode(data->wrapModeW));
	glSamplerParameteri(data->sampler, GL_TEXTURE_COMPARE_MODE, ConvertTextureCompareMode(data->compareMode));
	glSamplerParameteri(data->sampler, GL_TEXTURE_COMPARE_FUNC, ConvertDepthCompareFunc(data->compareFunc));
}
*/
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
/*
void RenderDeviceOpenGL::UseShaderProgram(unsigned int shaderProgram)
{
	glUseProgram(shaderProgram);
}
*/
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

/*
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
*/
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

/*
void RenderDeviceOpenGL::SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data)
{
	glVertexAttribPointer(data->attributeIndex, data->elementCount, ConvertVertexElemType(data->elementType),
		GL_FALSE, data->stride, reinterpret_cast<void*>(data->offset));
}

void RenderDeviceOpenGL::BindVertexArray(unsigned int vertexArrayId)
{
	glBindVertexArray(vertexArrayId);
}

void RenderDeviceOpenGL::Draw(RenderPrimitiveMode mode, int offset, int vertexCount)
{
	glDrawArrays(ConvertPrimitiveMode(mode), offset, vertexCount);
}

void RenderDeviceOpenGL::DrawIndexed(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType)
{
	glDrawElements(ConvertPrimitiveMode(mode), indexCount, ConvertIndexType(indexType), nullptr);
}

void RenderDeviceOpenGL::DrawInstanced(RenderPrimitiveMode mode, int offset, int vertexCount, int instanceCount)
{
	glDrawArraysInstanced(ConvertPrimitiveMode(mode), offset, vertexCount, instanceCount);
}

void RenderDeviceOpenGL::DrawIndexedInstanced(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType, int instanceCount)
{
	glDrawElementsInstanced(ConvertPrimitiveMode(mode), indexCount, ConvertIndexType(indexType), nullptr, instanceCount);
}

void RenderDeviceOpenGL::DrawIndirect(RenderPrimitiveMode mode, intptr_t offset)
{
	glDrawArraysIndirect(ConvertPrimitiveMode(mode), reinterpret_cast<const void*>(offset));
}

void RenderDeviceOpenGL::DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset)
{
	glDrawElementsIndirect(ConvertPrimitiveMode(mode), ConvertIndexType(indexType), reinterpret_cast<const void*>(offset));
}
*/
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

/*
void RenderDeviceOpenGL::BindBuffer(RenderBufferTarget target, unsigned int buffer)
{
	glBindBuffer(ConvertBufferTarget(target), buffer);
}

void RenderDeviceOpenGL::BindBufferBase(RenderBufferTarget target, unsigned int bindingPoint, unsigned int buffer)
{
	glBindBufferBase(ConvertBufferTarget(target), bindingPoint, buffer);
}

void RenderDeviceOpenGL::BindBufferRange(const RenderCommandData::BindBufferRange* data)
{
	glBindBufferRange(ConvertBufferTarget(data->target), data->bindingPoint, data->buffer, data->offset, data->length);
}


void RenderDeviceOpenGL::DispatchCompute(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ)
{
	glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void RenderDeviceOpenGL::DispatchComputeIndirect(intptr_t offset)
{
	glDispatchComputeIndirect(offset);
}

void RenderDeviceOpenGL::MemoryBarrier(const RenderCommandData::MemoryBarrier& barrier)
{
	GLbitfield bits = 0;
	if (barrier.vertexAttribArray) bits |= GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT;
	if (barrier.elementArray) bits |= GL_ELEMENT_ARRAY_BARRIER_BIT;
	if (barrier.uniform) bits |= GL_UNIFORM_BARRIER_BIT;
	if (barrier.textureFetch) bits |= GL_TEXTURE_FETCH_BARRIER_BIT;
	if (barrier.shaderImageAccess) bits |= GL_SHADER_IMAGE_ACCESS_BARRIER_BIT;
	if (barrier.command) bits |= GL_COMMAND_BARRIER_BIT;
	if (barrier.pixelBuffer) bits |= GL_PIXEL_BUFFER_BARRIER_BIT;
	if (barrier.textureUpdate) bits |= GL_TEXTURE_UPDATE_BARRIER_BIT;
	if (barrier.bufferUpdate) bits |= GL_BUFFER_UPDATE_BARRIER_BIT;
	if (barrier.clientMappedBuffer) bits |= GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT;
	if (barrier.framebuffer) bits |= GL_FRAMEBUFFER_BARRIER_BIT;
	if (barrier.transformFeedback) bits |= GL_TRANSFORM_FEEDBACK_BARRIER_BIT;
	if (barrier.atomicCounter) bits |= GL_ATOMIC_COUNTER_BARRIER_BIT;
	if (barrier.shaderStorage) bits |= GL_SHADER_STORAGE_BARRIER_BIT;
	if (barrier.queryBuffer) bits |= GL_QUERY_BUFFER_BARRIER_BIT;

	glMemoryBarrier(bits);
}
*/