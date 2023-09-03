#pragma once

#include "Metal/Metal.hpp"
#include "Foundation/Foundation.hpp"

#include "Rendering/RenderDevice.hpp"

namespace kokko
{

namespace render
{

class DeviceMetal : public Device
{
private:
    NS::SharedPtr<NS::AutoreleasePool> pool;
    NS::SharedPtr<MTL::Device> device;
    NS::SharedPtr<MTL::CommandQueue> queue;

public:
    DeviceMetal();
    ~DeviceMetal();

    virtual kokko::NativeRenderDevice* GetNativeDevice() override;
    
    virtual kokko::CommandBuffer* CreateCommandBuffer(Allocator* allocator) override;

    virtual void GetIntegerValue(RenderDeviceParameter parameter, int* valueOut) override;

    virtual void SetDebugMessageCallback(DebugCallbackFn callback) override;
    virtual void SetObjectLabel(RenderObjectType type, unsigned int object, ConstStringView label) override;
    virtual void SetObjectPtrLabel(void* ptr, ConstStringView label) override;
    virtual void BeginDebugScope(uint32_t id, ConstStringView message) override;
    virtual void EndDebugScope() override;

    virtual void CreateFramebuffers(unsigned int count, FramebufferId* framebuffersOut) override;
    virtual void DestroyFramebuffers(unsigned int count, const FramebufferId* framebuffers) override;
    virtual void AttachFramebufferTexture(
        FramebufferId framebuffer,
        RenderFramebufferAttachment attachment,
        TextureId texture,
        int level) override;
    virtual void AttachFramebufferTextureLayer(
        FramebufferId framebuffer,
        RenderFramebufferAttachment attachment,
        TextureId texture,
        int level,
        int layer) override;
    virtual void SetFramebufferDrawBuffers(
        FramebufferId framebuffer,
        unsigned int count,
        const RenderFramebufferAttachment* buffers) override;
    virtual void ReadFramebufferPixels(int x, int y, int width, int height,
        RenderTextureBaseFormat format, RenderTextureDataType type, void* data) override;

    virtual void CreateTextures(RenderTextureTarget type, unsigned int count, TextureId* texturesOut) override;
    virtual void DestroyTextures(unsigned int count, const TextureId* textures) override;
    virtual void SetTextureStorage2D(
        TextureId texture,
        int levels,
        RenderTextureSizedFormat format,
        int width,
        int height) override;
    virtual void SetTextureSubImage2D(
        TextureId texture,
        int level,
        int xOffset,
        int yOffset,
        int width,
        int height,
        RenderTextureBaseFormat format,
        RenderTextureDataType type,
        const void* data) override;
    virtual void SetTextureSubImage3D(
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
        const void* data) override;
    virtual void GenerateTextureMipmaps(TextureId texture) override;

    virtual void CreateSamplers(uint32_t count, const RenderSamplerParameters* params, SamplerId* samplersOut) override;
    virtual void DestroySamplers(uint32_t count, const SamplerId* samplers) override;

    virtual unsigned int CreateShaderProgram() override;
    virtual void DestroyShaderProgram(unsigned int shaderProgram) override;
    virtual void AttachShaderStageToProgram(unsigned int shaderProgram, unsigned int shaderStage) override;
    virtual void LinkShaderProgram(unsigned int shaderProgram) override;
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

    virtual void CreateVertexArrays(uint32_t count, VertexArrayId* vertexArraysOut) override;
    virtual void DestroyVertexArrays(uint32_t count, const VertexArrayId* vertexArrays) override;
    virtual void EnableVertexAttribute(VertexArrayId va, uint32_t attributeIndex) override;
    virtual void SetVertexArrayIndexBuffer(VertexArrayId va, BufferId buffer) override;
    virtual void SetVertexArrayVertexBuffer(
        VertexArrayId va,
        uint32_t bindingIndex,
        BufferId buffer,
        intptr_t offset,
        uint32_t stride) override;
    virtual void SetVertexAttribFormat(
        VertexArrayId va,
        uint32_t attributeIndex,
        uint32_t size,
        RenderVertexElemType elementType,
        uint32_t offset) override;
    virtual void SetVertexAttribBinding(
        VertexArrayId va,
        uint32_t attributeIndex,
        uint32_t bindingIndex) override;

    virtual void CreateBuffers(unsigned int count, BufferId* buffersOut) override;
    virtual void DestroyBuffers(unsigned int count, const BufferId* buffers) override;
    virtual void SetBufferStorage(
        BufferId buffer, unsigned int size, const void* data, BufferStorageFlags flags) override;
    virtual void SetBufferSubData(
        BufferId buffer, unsigned int offset, unsigned int size, const void* data) override;
    virtual void* MapBufferRange(
        BufferId buffer, intptr_t offset, size_t length, BufferMapFlags flags) override;
    virtual void UnmapBuffer(BufferId buffer) override;
};

}

}
