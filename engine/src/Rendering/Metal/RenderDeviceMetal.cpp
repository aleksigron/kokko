#include "Rendering/Metal/RenderDeviceMetal.hpp"

#include <cassert>

#include "Metal/Metal.hpp"

#include "Memory/Allocator.hpp"

#include "Rendering/Metal/CommandBufferMetal.hpp"

namespace kokko
{

namespace render
{

DeviceMetal::DeviceMetal()
{
    pool = NS::TransferPtr(NS::AutoreleasePool::alloc()->init());

    device = NS::TransferPtr(MTL::CreateSystemDefaultDevice());
    assert(device);
    queue = NS::TransferPtr(device->newCommandQueue());
    assert(queue);
}

DeviceMetal::~DeviceMetal()
{
}

NativeRenderDevice* DeviceMetal::GetNativeDevice()
{
    return reinterpret_cast<NativeRenderDevice*>(device.get());
}

kokko::CommandBuffer* DeviceMetal::CreateCommandBuffer(Allocator* allocator)
{
    return allocator->MakeNew<kokko::CommandBufferMetal>(queue.get());
}

void DeviceMetal::GetIntegerValue(RenderDeviceParameter parameter, int* valueOut)
{
    //glGetIntegerv(ConvertDeviceParameter(parameter), valueOut);
}

void DeviceMetal::SetDebugMessageCallback(DebugCallbackFn callback)
{
    //debugUserData.callback = callback;
    //glDebugMessageCallback(DebugMessageCallback, &debugUserData);
}

void DeviceMetal::SetObjectLabel(RenderObjectType type, unsigned int object, kokko::ConstStringView label)
{
    //glObjectLabel(ConvertObjectType(type), object, static_cast<GLsizei>(label.len), label.str);
}

void DeviceMetal::SetObjectPtrLabel(void* ptr, kokko::ConstStringView label)
{
    //glObjectPtrLabel(ptr, static_cast<GLsizei>(label.len), label.str);
}

void DeviceMetal::BeginDebugScope(uint32_t id, ConstStringView message)
{
    //glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, static_cast<uint32_t>(message.len), message.str);
}

void DeviceMetal::EndDebugScope()
{
    //glPopDebugGroup();
}

void DeviceMetal::CreateFramebuffers(unsigned int count, FramebufferId* framebuffersOut)
{
    // glCreateFramebuffers(count, &framebuffersOut[0].i);
}

void DeviceMetal::DestroyFramebuffers(unsigned int count, const FramebufferId* framebuffers)
{
    // glDeleteFramebuffers(count, &framebuffers[0].i);
}

void DeviceMetal::AttachFramebufferTexture(
    FramebufferId framebuffer,
    RenderFramebufferAttachment attachment,
    TextureId texture,
    int level)
{
    // glNamedFramebufferTexture(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level);
}

void DeviceMetal::AttachFramebufferTextureLayer(
    FramebufferId framebuffer,
    RenderFramebufferAttachment attachment,
    TextureId texture,
    int level,
    int layer)
{
    // glNamedFramebufferTextureLayer(framebuffer.i, ConvertFramebufferAttachment(attachment), texture.i, level, layer);
}

void DeviceMetal::SetFramebufferDrawBuffers(
    FramebufferId framebuffer,
    unsigned int count,
    const RenderFramebufferAttachment* buffers)
{
//    unsigned int attachments[16];
//
//    for (unsigned int i = 0; i < count; ++i)
//        attachments[i] = ConvertFramebufferAttachment(buffers[i]);
//
//    glNamedFramebufferDrawBuffers(framebuffer.i, count, attachments);
}

void DeviceMetal::ReadFramebufferPixels(int x, int y, int width, int height,
    RenderTextureBaseFormat format, RenderTextureDataType type, void* data)
{
    //glReadPixels(x, y, width, height, ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

// TEXTURE

void DeviceMetal::CreateTextures(
    RenderTextureTarget type,
    unsigned int count,
    TextureId* texturesOut)
{
    //glCreateTextures(ConvertTextureTarget(type), count, &texturesOut[0].i);
}

void DeviceMetal::DestroyTextures(unsigned int count, const TextureId* textures)
{
    //glDeleteTextures(count, &textures[0].i);
}

void DeviceMetal::SetTextureStorage2D(
    TextureId texture,
    int levels,
    RenderTextureSizedFormat format,
    int width,
    int height)
{
    //assert(levels > 0 && width > 0 && height > 0);
    //glTextureStorage2D(texture.i, levels, ConvertTextureSizedFormat(format), width, height);
}

void DeviceMetal::SetTextureSubImage2D(
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
    //glTextureSubImage2D(texture.i, level, xOffset, yOffset, width, height,
    //    ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

void DeviceMetal::SetTextureSubImage3D(
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
    //glTextureSubImage3D(texture.i, level, xoffset, yoffset, zoffset, width, height, depth,
    //    ConvertTextureBaseFormat(format), ConvertTextureDataType(type), data);
}

void DeviceMetal::GenerateTextureMipmaps(TextureId texture)
{
    //glGenerateTextureMipmap(texture.i);
}

// SAMPLERS

void DeviceMetal::CreateSamplers(
    uint32_t count, const RenderSamplerParameters* params, SamplerId* samplersOut)
{
//    glGenSamplers(count, &samplersOut[0].i);
//
//    for (uint32_t i = 0; i < count; ++i)
//    {
//        unsigned int sampler = samplersOut[i].i;
//        const RenderSamplerParameters& data = params[i];
//        glSamplerParameteri(sampler, GL_TEXTURE_MIN_FILTER, ConvertTextureFilterMode(data.minFilter));
//        glSamplerParameteri(sampler, GL_TEXTURE_MAG_FILTER, ConvertTextureFilterMode(data.magFilter));
//        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_S, ConvertTextureWrapMode(data.wrapModeU));
//        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_T, ConvertTextureWrapMode(data.wrapModeV));
//        glSamplerParameteri(sampler, GL_TEXTURE_WRAP_R, ConvertTextureWrapMode(data.wrapModeW));
//        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_MODE, ConvertTextureCompareMode(data.compareMode));
//        glSamplerParameteri(sampler, GL_TEXTURE_COMPARE_FUNC, ConvertDepthCompareFunc(data.compareFunc));
//    }
}

void DeviceMetal::DestroySamplers(uint32_t count, const SamplerId* samplers)
{
//    glDeleteSamplers(count, &samplers[0].i);
}

// SHADER PROGRAM

unsigned int DeviceMetal::CreateShaderProgram()
{
    //return glCreateProgram();
}

void DeviceMetal::DestroyShaderProgram(unsigned int shaderProgram)
{
    //glDeleteProgram(shaderProgram);
}

void DeviceMetal::AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage)
{
    //glAttachShader(shaderProgram, shaderStage);
}

void DeviceMetal::LinkShaderProgram(unsigned int shaderProgram)
{
    //glLinkProgram(shaderProgram);
}

int DeviceMetal::GetShaderProgramParameterInt(unsigned int shaderProgram, unsigned int parameter)
{
    int value = 0;
    //glGetProgramiv(shaderProgram, parameter, &value);
    return value;
}

bool DeviceMetal::GetShaderProgramLinkStatus(unsigned int shaderProgram)
{
    //return GetShaderProgramParameterInt(shaderProgram, GL_LINK_STATUS) == GL_TRUE;
}

int DeviceMetal::GetShaderProgramInfoLogLength(unsigned int shaderProgram)
{
    //return GetShaderProgramParameterInt(shaderProgram, GL_INFO_LOG_LENGTH);
}

void DeviceMetal::GetShaderProgramInfoLog(unsigned int shaderProgram, unsigned int maxLength, char* logOut)
{
    //glGetProgramInfoLog(shaderProgram, maxLength, nullptr, logOut);
}

// SHADER STAGE

unsigned int DeviceMetal::CreateShaderStage(RenderShaderStage stage)
{
    //return glCreateShader(ConvertShaderStage(stage));
}

void DeviceMetal::DestroyShaderStage(unsigned int shaderStage)
{
    //glDeleteShader(shaderStage);
}

void DeviceMetal::SetShaderStageSource(unsigned int shaderStage, const char* source, int length)
{
    //glShaderSource(shaderStage, 1, &source, &length);
}

void DeviceMetal::CompileShaderStage(unsigned int shaderStage)
{
    //glCompileShader(shaderStage);
}

int DeviceMetal::GetShaderStageParameterInt(unsigned int shaderStage, unsigned int parameter)
{
    int value = 0;
    //glGetShaderiv(shaderStage, parameter, &value);
    return value;
}

bool DeviceMetal::GetShaderStageCompileStatus(unsigned int shaderStage)
{
    //return GetShaderStageParameterInt(shaderStage, GL_COMPILE_STATUS) == GL_TRUE;
    return false;
}

int DeviceMetal::GetShaderStageInfoLogLength(unsigned int shaderStage)
{
    //return GetShaderStageParameterInt(shaderStage, GL_INFO_LOG_LENGTH);
    return 0;
}

void DeviceMetal::GetShaderStageInfoLog(unsigned int shaderStage, unsigned int maxLength, char* logOut)
{
    //glGetShaderInfoLog(shaderStage, maxLength, nullptr, logOut);
}

// UNIFORM

int DeviceMetal::GetUniformLocation(unsigned int shaderProgram, const char* uniformName)
{
    //return glGetUniformLocation(shaderProgram, uniformName);
    return 0;
}

// VERTEX ARRAY

void DeviceMetal::CreateVertexArrays(uint32_t count, VertexArrayId* vertexArraysOut)
{
//    glCreateVertexArrays(count, &vertexArraysOut[0].i);
}

void DeviceMetal::DestroyVertexArrays(uint32_t count, const VertexArrayId* vertexArrays)
{
//    glDeleteVertexArrays(count, &vertexArrays[0].i);
}

void DeviceMetal::EnableVertexAttribute(VertexArrayId va, uint32_t attributeIndex)
{
//    glEnableVertexArrayAttrib(va.i, attributeIndex);
}

void DeviceMetal::SetVertexArrayIndexBuffer(VertexArrayId va, BufferId buffer)
{
//    glVertexArrayElementBuffer(va.i, buffer.i);
}

void DeviceMetal::SetVertexArrayVertexBuffer(
    VertexArrayId va,
    uint32_t bindingIndex,
    BufferId buffer,
    intptr_t offset,
    uint32_t stride)
{
//    glVertexArrayVertexBuffer(va.i, bindingIndex, buffer.i, offset, stride);
}

void DeviceMetal::SetVertexAttribFormat(
    VertexArrayId va,
    uint32_t attributeIndex,
    uint32_t size,
    RenderVertexElemType elementType,
    uint32_t offset)
{
//    glVertexArrayAttribFormat(va.i, attributeIndex, size, ConvertVertexElemType(elementType), GL_FALSE, offset);
}

void DeviceMetal::SetVertexAttribBinding(
    VertexArrayId va,
    uint32_t attributeIndex,
    uint32_t bindingIndex)
{
//    glVertexArrayAttribBinding(va.i, attributeIndex, bindingIndex);
}

// BUFFERS

void DeviceMetal::CreateBuffers(unsigned int count, BufferId* buffersOut)
{
//    glCreateBuffers(count, &buffersOut[0].i);
}

void DeviceMetal::DestroyBuffers(unsigned int count, const BufferId* buffers)
{
//    glDeleteBuffers(count, &buffers[0].i);
}

void DeviceMetal::SetBufferStorage(
    BufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags)
{
//    GLbitfield bits = 0;
//    if (flags.dynamicStorage) bits |= GL_DYNAMIC_STORAGE_BIT;
//    if (flags.mapReadAccess) bits |= GL_MAP_READ_BIT;
//    if (flags.mapWriteAccess) bits |= GL_MAP_WRITE_BIT;
//    if (flags.mapPersistent) bits |= GL_MAP_PERSISTENT_BIT;
//    if (flags.mapCoherent) bits |= GL_MAP_COHERENT_BIT;
//
//    glNamedBufferStorage(buffer.i, size, data, bits);
}

void DeviceMetal::SetBufferSubData(
    BufferId buffer,
    unsigned int offset,
    unsigned int size,
    const void* data)
{
//    glNamedBufferSubData(buffer.i, offset, size, data);
}

void* DeviceMetal::MapBufferRange(
    BufferId buffer,
    intptr_t offset,
    size_t length,
    BufferMapFlags flags)
{
//    GLbitfield bits = 0;
//    if (flags.readAccess) bits |= GL_MAP_READ_BIT;
//    if (flags.writeAccess) bits |= GL_MAP_WRITE_BIT;
//    if (flags.invalidateRange) bits |= GL_MAP_INVALIDATE_RANGE_BIT;
//    if (flags.invalidateBuffer) bits |= GL_MAP_INVALIDATE_BUFFER_BIT;
//    if (flags.flushExplicit) bits |= GL_MAP_FLUSH_EXPLICIT_BIT;
//    if (flags.unsynchronized) bits |= GL_MAP_UNSYNCHRONIZED_BIT;
//    if (flags.persistent) bits |= GL_MAP_PERSISTENT_BIT;
//    if (flags.coherent) bits |= GL_MAP_COHERENT_BIT;
//
//    return glMapNamedBufferRange(buffer.i, offset, length, bits);
}

void DeviceMetal::UnmapBuffer(BufferId buffer)
{
//    glUnmapNamedBuffer(buffer.i);
}

} // namespace render

} // namespace kokko
