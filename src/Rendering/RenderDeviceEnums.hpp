#pragma once

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
	UniformBuffer
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
	FragmentShader
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
