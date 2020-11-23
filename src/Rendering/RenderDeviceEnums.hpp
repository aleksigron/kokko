#pragma once

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

enum class RenderFramebufferTarget
{
	Framebuffer
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