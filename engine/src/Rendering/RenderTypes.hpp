#pragma once

#include <cstdint>

#include "Math/Vec4.hpp"

namespace kokko
{

enum class AttachmentLoadAction : uint8_t
{
    DontCare,
    Load,
    Clear
};

enum class AttachmentStoreAction : uint8_t
{
    DontCare,
    Store
};

struct TextureHandle
{
    uint64_t storage;
};

struct RenderPassAttachment
{
    TextureHandle texture;
    AttachmentLoadAction loadAction;
    AttachmentStoreAction storeAction;
};

struct RenderPassColorAttachment : public RenderPassAttachment
{
    Vec4f clearColor;
};

struct RenderPassDepthAttachment : public RenderPassAttachment
{
    float clearDepth;
};

struct RenderPassStencilAttachment : public RenderPassAttachment
{
    uint32_t clearStencil;
};

}

enum class RenderDeviceParameter
{
	MaxUniformBlockSize,
	UniformBufferOffsetAlignment
};

enum class RenderDebugSource
{
	Api,
	WindowSystem,
	ShaderCompiler,
	ThirdParty,
	Application,
	Other
};

enum class RenderDebugType
{
	Error,
	DeprecatedBehavior,
	UndefinedBehavior,
	Portability,
	Performance,
	Marker,
	PushGroup,
	PopGroup,
	Other
};

enum class RenderDebugSeverity
{
	High,
	Medium,
	Low,
	Notification
};

enum class RenderObjectType
{
	Buffer,
	Shader,
	Program,
	VertexArray,
	Query,
	ProgramPipeline,
	TransformFeedback,
	Sampler,
	Texture,
	Renderbuffer,
	Framebuffer
};

enum class RenderClipOriginMode
{
	LowerLeft,
	UpperLeft
};

enum class RenderClipDepthMode
{
	NegativeOneToOne,
	ZeroToOne
};

enum class RenderCullFace
{
	None,
	Front,
	Back,
	FrontAndBack
};

enum class RenderBufferUsage
{
	StreamDraw,
	StreamRead,
	StreamCopy,
	StaticDraw,
	StaticRead,
	StaticCopy,
	DynamicDraw,
	DynamicRead,
	DynamicCopy
};

enum class RenderBufferTarget
{
	VertexBuffer,
	IndexBuffer,
	UniformBuffer,
	ShaderStorageBuffer,
	DrawIndirectBuffer,
	DispatchIndirectBuffer
};

enum class RenderBufferAccess
{
	ReadOnly,
	WriteOnly,
	ReadWrite
};

enum class RenderTextureTarget
{
	Texture1d,
	Texture2d,
	Texture3d,
	Texture1dArray,
	Texture2dArray,
	TextureCubeMap,
	TextureCubeMap_PositiveX,
	TextureCubeMap_NegativeX,
	TextureCubeMap_PositiveY,
	TextureCubeMap_NegativeY,
	TextureCubeMap_PositiveZ,
	TextureCubeMap_NegativeZ
};

enum class RenderTextureDataType
{
	UnsignedByte,
	SignedByte,
	UnsignedShort,
	SignedShort,
	UnsignedInt,
	SignedInt,
	Float
};

enum class RenderTextureBaseFormat
{
	R,
	RG,
	RGB,
	RGBA,
	Depth,
	DepthStencil
};

enum class RenderTextureSizedFormat
{
	R8,
	RG8,
	RGB8,
	RGBA8,
	SRGB8,
	SRGB8_A8,
	R16,
	RG16,
	RGB16,
	RGBA16,
	R16F,
	RG16F,
	RGB16F,
	RGBA16F,
	R32F,
	RG32F,
	RGB32F,
	RGBA32F,
	D32F,
	D24,
	D16,
	D32F_S8,
	D24_S8,
	STENCIL_INDEX8
};

enum class RenderFramebufferTarget
{
	Framebuffer
};

enum class RenderFramebufferAttachment
{
	None,
	Color0,
	Color1,
	Color2,
	Color3,
	Depth,
	DepthStencil
};

enum class RenderBlendFactor
{
	Zero,
	One,
	SrcColor,
	OneMinusSrcColor,
	DstColor,
	OneMinusDstColor,
	SrcAlpha,
	OneMinusSrcAlpha,
	DstAlpha,
	OneMinusDstAlpha,
	ConstantColor,
	OneMinusConstantColor,
	ConstantAlpha,
	OneMinusConstantAlpha,
	SrcAlphaSaturate,
	Src1Color,
	OneMinusSrc1Color,
	Src1Alpha,
	OneMinusSrc1Alpha
};

enum class RenderBlendEquation
{
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max
};

enum class RenderIndexType
{
	None,
	UnsignedByte,
	UnsignedShort,
	UnsignedInt
};

enum class RenderPrimitiveMode
{
	Points,
	LineStrip,
	LineLoop,
	Lines,
	TriangleStrip,
	TriangleFan,
	Triangles
};

enum class RenderShaderStage
{
	VertexShader,
	GeometryShader,
	FragmentShader,
	ComputeShader
};

enum class RenderTextureParameter
{
	MinificationFilter,
	MagnificationFilter,
	WrapModeU,
	WrapModeV,
	WrapModeW,
	CompareMode,
	CompareFunc,
};

enum class RenderTextureFilterMode
{
	Nearest,
	Linear,
	LinearMipmap
};

enum class RenderTextureWrapMode
{
	Repeat,
	MirroredRepeat,
	ClampToEdge,
};

enum class RenderTextureCompareMode
{
	None,
	CompareRefToTexture
};

enum class RenderDepthCompareFunc
{
	LessThanOrEqual,
	GreaterThanOrEqual,
	Less,
	Greater,
	Equal,
	NotEqual,
	Always,
	Never
};

enum class RenderVertexElemType
{
	Float
};

struct ClearMask
{
	bool color;
	bool depth;
	bool stencil;
};

struct RenderSamplerParameters
{
	RenderTextureFilterMode minFilter;
	RenderTextureFilterMode magFilter;
	RenderTextureWrapMode wrapModeU;
	RenderTextureWrapMode wrapModeV;
	RenderTextureWrapMode wrapModeW;
	RenderTextureCompareMode compareMode;
	RenderDepthCompareFunc compareFunc;
};

struct BufferStorageFlags
{
	bool dynamicStorage : 1; // Client can update data after creation using SetBufferSubData
	bool mapReadAccess : 1; // Client can map data for read access
	bool mapWriteAccess : 1; // Client can map data for write access
	bool mapPersistent : 1; // Client can use the buffer for other commands while it is mapped
	bool mapCoherent : 1; // While persistently mapped, changes to data are coherent

	static BufferStorageFlags None;
	static BufferStorageFlags Dynamic;
};

struct BufferMapFlags
{
	bool readAccess : 1; // Data can be read
	bool writeAccess : 1; // Data can be written
	bool invalidateRange : 1; // Previous contents of range can be discarded
	bool invalidateBuffer : 1; // Previous contents of buffer can be discarded
	bool flushExplicit : 1; // Changes are not implicitly flushed by unmap
	bool unsynchronized : 1; // No synchronization of other operations is attempted while mapped
	bool persistent : 1; // Mapping is to be made in a persistent fashion
	bool coherent : 1; // Persistent mapping is also to be coherent
};

struct MemoryBarrierFlags
{
	bool vertexAttribArray : 1;
	bool elementArray : 1;
	bool uniform : 1;
	bool textureFetch : 1;
	bool shaderImageAccess : 1;
	bool command : 1;
	bool pixelBuffer : 1;
	bool textureUpdate : 1;
	bool bufferUpdate : 1;
	bool clientMappedBuffer : 1;
	bool framebuffer : 1;
	bool transformFeedback : 1;
	bool atomicCounter : 1;
	bool shaderStorage : 1;
	bool queryBuffer : 1;
};
