#include "Rendering/Metal/RenderDeviceMetal.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Metal/CommandBufferMetal.hpp"

#include <cassert>

namespace kokko
{

RenderDeviceMetal::RenderDeviceMetal()
{
    device = NS::TransferPtr(MTL::CreateSystemDefaultDevice());
    assert(device);
    queue = NS::TransferPtr(device->newCommandQueue());
    assert(queue);

    pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());
}

RenderDeviceMetal::~RenderDeviceMetal()
{

}

NativeRenderDevice* RenderDeviceMetal::GetNativeDevice()
{
    return reinterpret_cast<NativeRenderDevice*>(device.get());
}

kokko::CommandBuffer* RenderDeviceMetal::CreateCommandBuffer(Allocator* allocator)
{
    return allocator->MakeNew<kokko::CommandBufferMetal>(this, queue->commandBuffer());
}

void RenderDeviceMetal::SetDebugMessageCallback(DebugCallbackFn callback)
{
	//debugUserData.callback = callback;
	//glDebugMessageCallback(DebugMessageCallback, &debugUserData);
}

void RenderDeviceMetal::SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label)
{
	//glObjectLabel(ConvertObjectType(type), object, static_cast<GLsizei>(label.len), label.str);
}

void RenderDeviceMetal::SetObjectPtrLabel(void* ptr, kokko::ConstStringView label)
{
	//glObjectPtrLabel(ptr, static_cast<GLsizei>(label.len), label.str);
}

void RenderDeviceMetal::GetIntegerValue(RenderDeviceParameter parameter, int* valueOut)
{
	//glGetIntegerv(ConvertDeviceParameter(parameter), valueOut);
}

void RenderDeviceMetal::PushDebugGroup(unsigned int id, kokko::ConstStringView message)
{
	//glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<GLsizei>(message.len), message.str);
}

void RenderDeviceMetal::PopDebugGroup()
{
	//glPopDebugGroup();
}

void RenderDeviceMetal::Clear(const RenderCommandData::ClearMask* data)
{
	unsigned int mask = 0;
/*
	if (data->color) mask |= GL_COLOR_BUFFER_BIT;
	if (data->depth) mask |= GL_DEPTH_BUFFER_BIT;
	if (data->stencil) mask |= GL_STENCIL_BUFFER_BIT;

	glClear(mask);*/
}

void RenderDeviceMetal::ClearColor(const RenderCommandData::ClearColorData* data)
{
	//glClearColor(data->r, data->g, data->b, data->a);
}

void RenderDeviceMetal::ClearDepth(float depth)
{
	//glClearDepth(depth);
}

void RenderDeviceMetal::BlendingEnable()
{
	//glEnable(GL_BLEND);
}

void RenderDeviceMetal::BlendingDisable()
{
	//glDisable(GL_BLEND);
}

void RenderDeviceMetal::BlendFunction(const RenderCommandData::BlendFunctionData* data)
{
	//BlendFunction(data->srcFactor, data->dstFactor);
}

void RenderDeviceMetal::BlendFunction(RenderBlendFactor srcFactor, RenderBlendFactor dstFactor)
{
	//glBlendFunc(ConvertBlendFactor(srcFactor), ConvertBlendFactor(dstFactor));
}

void RenderDeviceMetal::CubemapSeamlessEnable()
{
	//glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void RenderDeviceMetal::CubemapSeamlessDisable()
{
	//glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void RenderDeviceMetal::SetClipBehavior(RenderClipOriginMode origin, RenderClipDepthMode depth)
{
	//glClipControl(ConvertClipOriginMode(origin), ConvertClipDepthMode(depth));
}

void RenderDeviceMetal::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	//glDepthRange(data->near, data->far);
}

void RenderDeviceMetal::Viewport(const RenderCommandData::ViewportData* data)
{
	//glViewport(data->x, data->y, data->w, data->h);
}

void RenderDeviceMetal::ScissorTestEnable()
{
	//glEnable(GL_SCISSOR_TEST);
}

void RenderDeviceMetal::ScissorTestDisable()
{
	//glDisable(GL_SCISSOR_TEST);
}

void RenderDeviceMetal::DepthTestEnable()
{
	//glEnable(GL_DEPTH_TEST);
}

void RenderDeviceMetal::DepthTestDisable()
{
	//glDisable(GL_DEPTH_TEST);
}

void RenderDeviceMetal::DepthTestFunction(RenderDepthCompareFunc function)
{
	//glDepthFunc(ConvertDepthCompareFunc(function));
}

void RenderDeviceMetal::DepthWriteEnable()
{
	//glDepthMask(GL_TRUE);
}

void RenderDeviceMetal::DepthWriteDisable()
{
	//glDepthMask(GL_FALSE);
}

// CULL FACE

void RenderDeviceMetal::CullFaceEnable()
{
	//glEnable(GL_CULL_FACE);
}

void RenderDeviceMetal::CullFaceDisable()
{
	//glDisable(GL_CULL_FACE);
}

void RenderDeviceMetal::CullFaceFront()
{
	//glCullFace(GL_FRONT);
}

void RenderDeviceMetal::CullFaceBack()
{
	//glCullFace(GL_BACK);
}

// FRAMEBUFFER

void RenderDeviceMetal::FramebufferSrgbEnable()
{
	//glEnable(GL_FRAMEBUFFER_SRGB);
}

void RenderDeviceMetal::FramebufferSrgbDisable()
{
	//glDisable(GL_FRAMEBUFFER_SRGB);
}

void RenderDeviceMetal::CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut)
{
	//glGenFramebuffers(count, framebuffersOut);
}

void RenderDeviceMetal::DestroyFramebuffers(unsigned int count, unsigned int* framebuffers)
{
	//glDeleteFramebuffers(count, framebuffers);
}

void RenderDeviceMetal::BindFramebuffer(const RenderCommandData::BindFramebufferData* data)
{
	//BindFramebuffer(data->target, data->framebuffer);
}

void RenderDeviceMetal::BindFramebuffer(RenderFramebufferTarget target, unsigned int framebuffer)
{
	//glBindFramebuffer(ConvertFramebufferTarget(target), framebuffer);
}

void RenderDeviceMetal::AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data)
{
	//glFramebufferTexture2D(ConvertFramebufferTarget(data->target), ConvertFramebufferAttachment(data->attachment),
	//	ConvertTextureTarget(data->textureTarget), data->texture, data->mipLevel);
}

void RenderDeviceMetal::SetFramebufferDrawBuffers(unsigned int count, const RenderFramebufferAttachment* buffers)
{
	unsigned int attachments[16];

	//for (unsigned int i = 0; i < count; ++i)
    //  attachments[i] = ConvertFramebufferAttachment(buffers[i]);

	//glDrawBuffers(count, attachments);
}

// TEXTURE

void RenderDeviceMetal::CreateTextures(unsigned int count, unsigned int* texturesOut)
{
	//glGenTextures(count, texturesOut);
}

void RenderDeviceMetal::DestroyTextures(unsigned int count, unsigned int* textures)
{
	//glDeleteTextures(count, textures);
}

void RenderDeviceMetal::BindTexture(RenderTextureTarget target, unsigned int texture)
{
	//glBindTexture(ConvertTextureTarget(target), texture);
}

void RenderDeviceMetal::SetTextureStorage2D(const RenderCommandData::SetTextureStorage2D* data)
{
	//glTexStorage2D(ConvertTextureTarget(data->target), data->levels,
	//	ConvertTextureSizedFormat(data->format), data->width, data->height);
}

void RenderDeviceMetal::SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data)
{
	//glTexImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->internalFormat,
	//	data->width, data->height, 0, data->format, data->type, data->data);
}

void RenderDeviceMetal::SetTextureSubImage2D(const RenderCommandData::SetTextureSubImage2D* data)
{
	//glTexSubImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->xOffset, data->yOffset,
	//	data->width, data->height, ConvertTextureBaseFormat(data->format),
	//	ConvertTextureDataType(data->type), data->data);
}

void RenderDeviceMetal::SetTextureImageCompressed2D(const RenderCommandData::SetTextureImageCompressed2D* data)
{
	//glCompressedTexImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->internalFormat,
	//	data->width, data->height, 0, data->dataSize, data->data);
}

void RenderDeviceMetal::GenerateTextureMipmaps(RenderTextureTarget target)
{
	//glGenerateMipmap(ConvertTextureTarget(target));
}

void RenderDeviceMetal::SetActiveTextureUnit(unsigned int textureUnit)
{
	//glActiveTexture(GL_TEXTURE0 + textureUnit);
}

void RenderDeviceMetal::SetTextureParameterInt(RenderTextureTarget target, RenderTextureParameter parameter, unsigned int value)
{
	//glTexParameteri(ConvertTextureTarget(target), ConvertTextureParameter(parameter), value);
}

void RenderDeviceMetal::SetTextureMinFilter(RenderTextureTarget target, RenderTextureFilterMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::MinificationFilter, ConvertTextureFilterMode(mode));
}

void RenderDeviceMetal::SetTextureMagFilter(RenderTextureTarget target, RenderTextureFilterMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::MagnificationFilter, ConvertTextureFilterMode(mode));
}

void RenderDeviceMetal::SetTextureWrapModeU(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::WrapModeU, ConvertTextureWrapMode(mode));
}

void RenderDeviceMetal::SetTextureWrapModeV(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::WrapModeV, ConvertTextureWrapMode(mode));
}

void RenderDeviceMetal::SetTextureWrapModeW(RenderTextureTarget target, RenderTextureWrapMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::WrapModeW, ConvertTextureWrapMode(mode));
}

void RenderDeviceMetal::SetTextureCompareMode(RenderTextureTarget target, RenderTextureCompareMode mode)
{
	//SetTextureParameterInt(target, RenderTextureParameter::CompareMode, ConvertTextureCompareMode(mode));
}

void RenderDeviceMetal::SetTextureCompareFunc(RenderTextureTarget target, RenderDepthCompareFunc func)
{
	//SetTextureParameterInt(target, RenderTextureParameter::CompareFunc, ConvertDepthCompareFunc(func));
}

void RenderDeviceMetal::CreateSamplers(unsigned int count, unsigned int* samplersOut)
{
	//glGenSamplers(count, samplersOut);
}

void RenderDeviceMetal::DestroySamplers(unsigned int count, unsigned int* samplers)
{
	//glDeleteSamplers(count, samplers);
}

void RenderDeviceMetal::BindSampler(unsigned int textureUnit, unsigned int sampler)
{
	//glBindSampler(textureUnit, sampler);
}

void RenderDeviceMetal::SetSamplerParameters(const RenderCommandData::SetSamplerParameters* data)
{
	/*glSamplerParameteri(data->sampler, GL_TEXTURE_MIN_FILTER, ConvertTextureFilterMode(data->minFilter));
	glSamplerParameteri(data->sampler, GL_TEXTURE_MAG_FILTER, ConvertTextureFilterMode(data->magFilter));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_S, ConvertTextureWrapMode(data->wrapModeU));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_T, ConvertTextureWrapMode(data->wrapModeV));
	glSamplerParameteri(data->sampler, GL_TEXTURE_WRAP_R, ConvertTextureWrapMode(data->wrapModeW));
	glSamplerParameteri(data->sampler, GL_TEXTURE_COMPARE_MODE, ConvertTextureCompareMode(data->compareMode));
	glSamplerParameteri(data->sampler, GL_TEXTURE_COMPARE_FUNC, ConvertDepthCompareFunc(data->compareFunc));*/
}

// SHADER PROGRAM

unsigned int RenderDeviceMetal::CreateShaderProgram()
{
	//return glCreateProgram();
}

void RenderDeviceMetal::DestroyShaderProgram(unsigned int shaderProgram)
{
	//glDeleteProgram(shaderProgram);
}

void RenderDeviceMetal::AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage)
{
	//glAttachShader(shaderProgram, shaderStage);
}

void RenderDeviceMetal::LinkShaderProgram(unsigned int shaderProgram)
{
	//glLinkProgram(shaderProgram);
}

void RenderDeviceMetal::UseShaderProgram(unsigned int shaderProgram)
{
	//glUseProgram(shaderProgram);
}

int RenderDeviceMetal::GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter)
{
	int value = 0;
	//glGetProgramiv(shaderProgram, parameter, &value);
	return value;
}

bool RenderDeviceMetal::GetShaderProgramLinkStatus(unsigned int shaderProgram)
{
	//return GetShaderProgramParameterInt(shaderProgram, GL_LINK_STATUS) == GL_TRUE;
}

int RenderDeviceMetal::GetShaderProgramInfoLogLength(unsigned int shaderProgram)
{
	//return GetShaderProgramParameterInt(shaderProgram, GL_INFO_LOG_LENGTH);
}

void RenderDeviceMetal::GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut)
{
	//glGetProgramInfoLog(shaderProgram, maxLength, nullptr, logOut);
}

// SHADER STAGE

unsigned int RenderDeviceMetal::CreateShaderStage(RenderShaderStage stage)
{
	//return glCreateShader(ConvertShaderStage(stage));
}

void RenderDeviceMetal::DestroyShaderStage(unsigned int shaderStage)
{
	//glDeleteShader(shaderStage);
}

void RenderDeviceMetal::SetShaderStageSource(unsigned int shaderStage, const char* source, int length)
{
	//glShaderSource(shaderStage, 1, &source, &length);
}

void RenderDeviceMetal::CompileShaderStage(unsigned int shaderStage)
{
	//glCompileShader(shaderStage);
}

int RenderDeviceMetal::GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter)
{
	int value = 0;
	//glGetShaderiv(shaderStage, parameter, &value);
	return value;
}

bool RenderDeviceMetal::GetShaderStageCompileStatus(unsigned int shaderStage)
{
	//return GetShaderStageParameterInt(shaderStage, GL_COMPILE_STATUS) == GL_TRUE;
    return false;
}

int RenderDeviceMetal::GetShaderStageInfoLogLength(unsigned int shaderStage)
{
	//return GetShaderStageParameterInt(shaderStage, GL_INFO_LOG_LENGTH);
    return 0;
}

void RenderDeviceMetal::GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut)
{
	//glGetShaderInfoLog(shaderStage, maxLength, nullptr, logOut);
}

// UNIFORM

int RenderDeviceMetal::GetUniformLocation(unsigned int shaderProgram, const char* uniformName)
{
	//return glGetUniformLocation(shaderProgram, uniformName);
    return 0;
}

void RenderDeviceMetal::SetUniformMat4x4f(int uniform, unsigned int count, const float* values)
{
	//glUniformMatrix4fv(uniform, count, GL_FALSE, values);
}

void RenderDeviceMetal::SetUniformVec4f(int uniform, unsigned int count, const float* values)
{
	//glUniform4fv(uniform, count, values);
}

void RenderDeviceMetal::SetUniformVec3f(int uniform, unsigned int count, const float* values)
{
	//glUniform3fv(uniform, count, values);
}

void RenderDeviceMetal::SetUniformVec2f(int uniform, unsigned int count, const float* values)
{
	//glUniform2fv(uniform, count, values);
}

void RenderDeviceMetal::SetUniformFloat(int uniform, float value)
{
	//glUniform1f(uniform, value);
}

void RenderDeviceMetal::SetUniformInt(int uniform, int value)
{
	//glUniform1i(uniform, value);
}

// VERTEX ARRAY

void RenderDeviceMetal::CreateVertexArrays(unsigned int count, unsigned int* vertexArraysOut)
{
	//glGenVertexArrays(count, vertexArraysOut);
}

void RenderDeviceMetal::DestroyVertexArrays(unsigned int count, unsigned int* vertexArrays)
{
	//glDeleteVertexArrays(count, vertexArrays);
}

void RenderDeviceMetal::BindVertexArray(unsigned int vertexArrayId)
{
	//glBindVertexArray(vertexArrayId);
}

void RenderDeviceMetal::EnableVertexAttribute(unsigned int index)
{
	//glEnableVertexAttribArray(index);
}

void RenderDeviceMetal::SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data)
{
	//glVertexAttribPointer(data->attributeIndex, data->elementCount, ConvertVertexElemType(data->elementType),
	//	GL_FALSE, data->stride, reinterpret_cast<void*>(data->offset));
}

void RenderDeviceMetal::Draw(RenderPrimitiveMode mode, int offset, int vertexCount)
{
	//glDrawArrays(ConvertPrimitiveMode(mode), offset, vertexCount);
}

void RenderDeviceMetal::DrawIndexed(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType)
{
	//glDrawElements(ConvertPrimitiveMode(mode), indexCount, ConvertIndexType(indexType), nullptr);
}

void RenderDeviceMetal::DrawInstanced(RenderPrimitiveMode mode, int offset, int vertexCount, int instanceCount)
{
	//glDrawArraysInstanced(ConvertPrimitiveMode(mode), offset, vertexCount, instanceCount);
}

void RenderDeviceMetal::DrawIndexedInstanced(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType, int instanceCount)
{
	//glDrawElementsInstanced(ConvertPrimitiveMode(mode), indexCount, ConvertIndexType(indexType), nullptr, instanceCount);
}

void RenderDeviceMetal::DrawIndirect(RenderPrimitiveMode mode, intptr_t offset)
{
	//glDrawArraysIndirect(ConvertPrimitiveMode(mode), reinterpret_cast<const void*>(offset));
}

void RenderDeviceMetal::DrawIndexedIndirect(RenderPrimitiveMode mode, RenderIndexType indexType, intptr_t offset)
{
	//glDrawElementsIndirect(ConvertPrimitiveMode(mode), ConvertIndexType(indexType), reinterpret_cast<const void*>(offset));
}

void RenderDeviceMetal::CreateBuffers(unsigned int count, unsigned int* buffersOut)
{
	//glGenBuffers(count, buffersOut);
}

void RenderDeviceMetal::DestroyBuffers(unsigned int count, unsigned int* buffers)
{
	//glDeleteBuffers(count, buffers);
}

void RenderDeviceMetal::BindBuffer(RenderBufferTarget target, unsigned int buffer)
{
	//glBindBuffer(ConvertBufferTarget(target), buffer);
}

void RenderDeviceMetal::BindBufferBase(RenderBufferTarget target, unsigned int bindingPoint, unsigned int buffer)
{
	//glBindBufferBase(ConvertBufferTarget(target), bindingPoint, buffer);
}

void RenderDeviceMetal::BindBufferRange(const RenderCommandData::BindBufferRange* data)
{
	//glBindBufferRange(ConvertBufferTarget(data->target), data->bindingPoint, data->buffer, data->offset, data->length);
}

void RenderDeviceMetal::SetBufferStorage(const RenderCommandData::SetBufferStorage* data)
{
	/*GLbitfield bits = 0;
	if (data->dynamicStorage) bits |= GL_DYNAMIC_STORAGE_BIT;
	if (data->mapReadAccess) bits |= GL_MAP_READ_BIT;
	if (data->mapWriteAccess) bits |= GL_MAP_WRITE_BIT;
	if (data->mapPersistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (data->mapCoherent) bits |= GL_MAP_COHERENT_BIT;

	glBufferStorage(ConvertBufferTarget(data->target), data->size, data->data, bits);*/
}

void RenderDeviceMetal::SetBufferData(RenderBufferTarget target, unsigned int size, const void* data, RenderBufferUsage usage)
{
	//glBufferData(ConvertBufferTarget(target), size, data, ConvertBufferUsage(usage));
}

void RenderDeviceMetal::SetBufferSubData(RenderBufferTarget target, unsigned int offset, unsigned int size, const void* data)
{
	//glBufferSubData(ConvertBufferTarget(target), offset, size, data);
}

void* RenderDeviceMetal::MapBuffer(RenderBufferTarget target, RenderBufferAccess access)
{
	//return glMapBuffer(ConvertBufferTarget(target), ConvertBufferAccess(access));
    return nullptr;
}

void* RenderDeviceMetal::MapBufferRange(const RenderCommandData::MapBufferRange* data)
{
	/*GLbitfield bits = 0;
	if (data->readAccess) bits |= GL_MAP_READ_BIT;
	if (data->writeAccess) bits |= GL_MAP_WRITE_BIT;
	if (data->invalidateRange) bits |= GL_MAP_INVALIDATE_RANGE_BIT;
	if (data->invalidateBuffer) bits |= GL_MAP_INVALIDATE_BUFFER_BIT;
	if (data->flushExplicit) bits |= GL_MAP_FLUSH_EXPLICIT_BIT;
	if (data->unsynchronized) bits |= GL_MAP_UNSYNCHRONIZED_BIT;
	if (data->persistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (data->coherent) bits |= GL_MAP_COHERENT_BIT;

	return glMapBufferRange(ConvertBufferTarget(data->target), data->offset, data->length, bits);*/
    return nullptr;
}

void RenderDeviceMetal::UnmapBuffer(RenderBufferTarget target)
{
	//glUnmapBuffer(ConvertBufferTarget(target));
}

void RenderDeviceMetal::DispatchCompute(unsigned int numGroupsX, unsigned int numGroupsY, unsigned int numGroupsZ)
{
	//glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

void RenderDeviceMetal::DispatchComputeIndirect(intptr_t offset)
{
	//glDispatchComputeIndirect(offset);
}

void RenderDeviceMetal::MemoryBarrier(const RenderCommandData::MemoryBarrier& barrier)
{
	/*GLbitfield bits = 0;
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

	glMemoryBarrier(bits);*/
}

} // namespace kokko
