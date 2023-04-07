#include "Rendering/RenderDeviceEnumsOpenGL.hpp"

#include "System/IncludeOpenGL.hpp"

uint32_t ConvertDeviceParameter(RenderDeviceParameter parameter)
{
	switch (parameter)
	{
	case RenderDeviceParameter::MaxUniformBlockSize: return GL_MAX_UNIFORM_BLOCK_SIZE;
	case RenderDeviceParameter::UniformBufferOffsetAlignment: return GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT;
	default: return 0;
	}
}

uint32_t ConvertClipOriginMode(RenderClipOriginMode origin)
{
	switch (origin)
	{
	case RenderClipOriginMode::LowerLeft: return GL_LOWER_LEFT;
	case RenderClipOriginMode::UpperLeft: return GL_UPPER_LEFT;
	default: return 0;
	}
}

uint32_t ConvertClipDepthMode(RenderClipDepthMode depth)
{
	switch (depth)
	{
	case RenderClipDepthMode::NegativeOneToOne: return GL_NEGATIVE_ONE_TO_ONE;
	case RenderClipDepthMode::ZeroToOne: return GL_ZERO_TO_ONE;
	default: return 0;
	}
}

uint32_t ConvertCullFace(RenderCullFace face)
{
	switch (face)
	{
	case RenderCullFace::Front: return GL_FRONT;
	case RenderCullFace::Back: return GL_BACK;
	case RenderCullFace::FrontAndBack: return GL_FRONT_AND_BACK;
	default: return 0;
	}
}

uint32_t ConvertBufferUsage(RenderBufferUsage usage)
{
	switch (usage)
	{
	case RenderBufferUsage::StreamDraw: return GL_STREAM_DRAW;
	case RenderBufferUsage::StreamRead: return GL_STREAM_READ;
	case RenderBufferUsage::StreamCopy: return GL_STREAM_COPY;
	case RenderBufferUsage::StaticDraw: return GL_STATIC_DRAW;
	case RenderBufferUsage::StaticRead: return GL_STATIC_READ;
	case RenderBufferUsage::StaticCopy: return GL_STATIC_COPY;
	case RenderBufferUsage::DynamicDraw: return GL_DYNAMIC_DRAW;
	case RenderBufferUsage::DynamicRead: return GL_DYNAMIC_READ;
	case RenderBufferUsage::DynamicCopy: return GL_DYNAMIC_COPY;
	default: return 0;
	}
}

uint32_t ConvertBufferTarget(RenderBufferTarget target)
{
	switch (target)
	{
	case RenderBufferTarget::VertexBuffer: return GL_ARRAY_BUFFER;
	case RenderBufferTarget::IndexBuffer: return GL_ELEMENT_ARRAY_BUFFER;
	case RenderBufferTarget::UniformBuffer: return GL_UNIFORM_BUFFER;
	case RenderBufferTarget::ShaderStorageBuffer: return GL_SHADER_STORAGE_BUFFER;
	case RenderBufferTarget::DrawIndirectBuffer: return GL_DRAW_INDIRECT_BUFFER;
	case RenderBufferTarget::DispatchIndirectBuffer: return GL_DISPATCH_INDIRECT_BUFFER;
	default: return 0;
	}
}

uint32_t ConvertBufferAccess(RenderBufferAccess access)
{
	switch (access)
	{
	case RenderBufferAccess::ReadOnly: return GL_READ_ONLY;
	case RenderBufferAccess::WriteOnly: return GL_WRITE_ONLY;
	case RenderBufferAccess::ReadWrite: return GL_READ_WRITE;
	default: return 0;
	}
}

uint32_t ConvertTextureTarget(RenderTextureTarget target)
{
	switch (target)
	{
	case RenderTextureTarget::Texture1d: return GL_TEXTURE_1D;
	case RenderTextureTarget::Texture2d: return GL_TEXTURE_2D;
	case RenderTextureTarget::Texture3d: return GL_TEXTURE_3D;
	case RenderTextureTarget::Texture1dArray: return GL_TEXTURE_1D_ARRAY;
	case RenderTextureTarget::Texture2dArray: return GL_TEXTURE_2D_ARRAY;
	case RenderTextureTarget::TextureCubeMap: return GL_TEXTURE_CUBE_MAP;
	case RenderTextureTarget::TextureCubeMap_PositiveX: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
	case RenderTextureTarget::TextureCubeMap_NegativeX: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
	case RenderTextureTarget::TextureCubeMap_PositiveY: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
	case RenderTextureTarget::TextureCubeMap_NegativeY: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
	case RenderTextureTarget::TextureCubeMap_PositiveZ: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
	case RenderTextureTarget::TextureCubeMap_NegativeZ: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
	default: return 0;
	}
}

uint32_t ConvertTextureDataType(RenderTextureDataType type)
{
	switch (type)
	{
	case RenderTextureDataType::UnsignedByte: return GL_UNSIGNED_BYTE;
	case RenderTextureDataType::SignedByte: return GL_BYTE;
	case RenderTextureDataType::UnsignedShort: return GL_UNSIGNED_SHORT;
	case RenderTextureDataType::SignedShort: return GL_SHORT;
	case RenderTextureDataType::UnsignedInt: return GL_UNSIGNED_INT;
	case RenderTextureDataType::SignedInt: return GL_INT;
	case RenderTextureDataType::Float: return GL_FLOAT;
	default: return 0;
	}
}

uint32_t ConvertTextureSizedFormat(RenderTextureSizedFormat format)
{
	switch (format)
	{
	case RenderTextureSizedFormat::R8: return GL_R8;
	case RenderTextureSizedFormat::RG8: return GL_RG8;
	case RenderTextureSizedFormat::RGB8: return GL_RGB8;
	case RenderTextureSizedFormat::RGBA8: return GL_RGBA8;
	case RenderTextureSizedFormat::SRGB8: return GL_SRGB8;
	case RenderTextureSizedFormat::SRGB8_A8: return GL_SRGB8_ALPHA8;
	case RenderTextureSizedFormat::R16: return GL_R16;
	case RenderTextureSizedFormat::RG16: return GL_RG16;
	case RenderTextureSizedFormat::RGB16: return GL_RGB16;
	case RenderTextureSizedFormat::RGBA16: return GL_RGBA16;
	case RenderTextureSizedFormat::R16F: return GL_R16F;
	case RenderTextureSizedFormat::RG16F: return GL_RG16F;
	case RenderTextureSizedFormat::RGB16F: return GL_RGB16F;
	case RenderTextureSizedFormat::RGBA16F: return GL_RGBA16F;
	case RenderTextureSizedFormat::R32F: return GL_R32F;
	case RenderTextureSizedFormat::RG32F: return GL_RG32F;
	case RenderTextureSizedFormat::RGB32F: return GL_RGB32F;
	case RenderTextureSizedFormat::RGBA32F: return GL_RGBA32F;
	case RenderTextureSizedFormat::D32F: return GL_DEPTH_COMPONENT32F;
	case RenderTextureSizedFormat::D24: return GL_DEPTH_COMPONENT24;
	case RenderTextureSizedFormat::D16: return GL_DEPTH_COMPONENT16;
	case RenderTextureSizedFormat::D32F_S8: return GL_DEPTH32F_STENCIL8;
	case RenderTextureSizedFormat::D24_S8: return GL_DEPTH24_STENCIL8;
	case RenderTextureSizedFormat::STENCIL_INDEX8: return GL_STENCIL_INDEX8;
	default: return 0;
	}
}

uint32_t ConvertTextureBaseFormat(RenderTextureBaseFormat format)
{
	switch (format)
	{
	case RenderTextureBaseFormat::R: return GL_RED;
	case RenderTextureBaseFormat::RG: return GL_RG;
	case RenderTextureBaseFormat::RGB: return GL_RGB;
	case RenderTextureBaseFormat::RGBA: return GL_RGBA;
	case RenderTextureBaseFormat::Depth: return GL_DEPTH_COMPONENT;
	case RenderTextureBaseFormat::DepthStencil: return GL_DEPTH_STENCIL;
	default: return 0;
	}
}

uint32_t ConvertTextureParameter(RenderTextureParameter parameter)
{
	switch (parameter)
	{
	case RenderTextureParameter::MinificationFilter: return GL_TEXTURE_MIN_FILTER;
	case RenderTextureParameter::MagnificationFilter: return GL_TEXTURE_MAG_FILTER;
	case RenderTextureParameter::WrapModeU: return GL_TEXTURE_WRAP_S;
	case RenderTextureParameter::WrapModeV: return GL_TEXTURE_WRAP_T;
	case RenderTextureParameter::WrapModeW: return GL_TEXTURE_WRAP_R;
	case RenderTextureParameter::CompareMode: return GL_TEXTURE_COMPARE_MODE;
	case RenderTextureParameter::CompareFunc: return GL_TEXTURE_COMPARE_FUNC;
	default: return 0;
	}
}

uint32_t ConvertTextureFilterMode(RenderTextureFilterMode mode)
{
	switch (mode)
	{
	case RenderTextureFilterMode::Nearest: return GL_NEAREST;
	case RenderTextureFilterMode::Linear: return GL_LINEAR;
	case RenderTextureFilterMode::LinearMipmap: return GL_LINEAR_MIPMAP_LINEAR;
	default: return 0;
	}
}

uint32_t ConvertTextureWrapMode(RenderTextureWrapMode mode)
{
	switch (mode)
	{
	case RenderTextureWrapMode::Repeat: return GL_REPEAT;
	case RenderTextureWrapMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
	case RenderTextureWrapMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
	default: return 0;
	}
}

uint32_t ConvertTextureCompareMode(RenderTextureCompareMode mode)
{
	switch (mode)
	{
	case RenderTextureCompareMode::None: return GL_NONE;
	case RenderTextureCompareMode::CompareRefToTexture: return GL_COMPARE_REF_TO_TEXTURE;
	default: return 0;
	}
}

uint32_t ConvertDepthCompareFunc(RenderDepthCompareFunc func)
{
	switch (func)
	{
	case RenderDepthCompareFunc::LessThanOrEqual: return GL_LEQUAL;
	case RenderDepthCompareFunc::GreaterThanOrEqual: return GL_GEQUAL;
	case RenderDepthCompareFunc::Less: return GL_LESS;
	case RenderDepthCompareFunc::Greater: return GL_GREATER;
	case RenderDepthCompareFunc::Equal: return GL_EQUAL;
	case RenderDepthCompareFunc::NotEqual: return GL_NOTEQUAL;
	case RenderDepthCompareFunc::Always: return GL_ALWAYS;
	case RenderDepthCompareFunc::Never: return GL_NEVER;
	default: return 0;
	}
}

uint32_t ConvertFramebufferTarget(RenderFramebufferTarget target)
{
	switch (target)
	{
	case RenderFramebufferTarget::Framebuffer: return GL_FRAMEBUFFER;
	default: return 0;
	}
}

uint32_t ConvertFramebufferAttachment(RenderFramebufferAttachment attachment)
{
	switch (attachment)
	{
	case RenderFramebufferAttachment::Color0: return GL_COLOR_ATTACHMENT0;
	case RenderFramebufferAttachment::Color1: return GL_COLOR_ATTACHMENT1;
	case RenderFramebufferAttachment::Color2: return GL_COLOR_ATTACHMENT2;
	case RenderFramebufferAttachment::Color3: return GL_COLOR_ATTACHMENT3;
	case RenderFramebufferAttachment::Depth: return GL_DEPTH_ATTACHMENT;
	case RenderFramebufferAttachment::DepthStencil: return GL_DEPTH_STENCIL_ATTACHMENT;
	default: return 0;
	}
}

uint32_t ConvertBlendFactor(RenderBlendFactor factor)
{
	switch (factor)
	{
	case RenderBlendFactor::Zero: return GL_ZERO;
	case RenderBlendFactor::One: return GL_ONE;
	case RenderBlendFactor::SrcColor: return GL_SRC_COLOR;
	case RenderBlendFactor::OneMinusSrcColor: return GL_ONE_MINUS_SRC_COLOR;
	case RenderBlendFactor::DstColor: return GL_DST_COLOR;
	case RenderBlendFactor::OneMinusDstColor: return GL_ONE_MINUS_DST_COLOR;
	case RenderBlendFactor::SrcAlpha: return GL_SRC_ALPHA;
	case RenderBlendFactor::OneMinusSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
	case RenderBlendFactor::DstAlpha: return GL_DST_ALPHA;
	case RenderBlendFactor::OneMinusDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
	case RenderBlendFactor::ConstantColor: return GL_CONSTANT_COLOR;
	case RenderBlendFactor::OneMinusConstantColor: return GL_ONE_MINUS_CONSTANT_COLOR;
	case RenderBlendFactor::ConstantAlpha: return GL_CONSTANT_ALPHA;
	case RenderBlendFactor::OneMinusConstantAlpha: return GL_ONE_MINUS_CONSTANT_ALPHA;
	case RenderBlendFactor::SrcAlphaSaturate: return GL_SRC_ALPHA_SATURATE;
	case RenderBlendFactor::Src1Color: return GL_SRC1_COLOR;
	case RenderBlendFactor::OneMinusSrc1Color: return GL_ONE_MINUS_SRC1_COLOR;
	case RenderBlendFactor::Src1Alpha: return GL_SRC1_ALPHA;
	case RenderBlendFactor::OneMinusSrc1Alpha: return GL_ONE_MINUS_SRC1_ALPHA;
	default: return 0;
	}
}

uint32_t ConvertIndexType(RenderIndexType type)
{
	switch (type)
	{
	case RenderIndexType::None: return 0;
	case RenderIndexType::UnsignedByte: return GL_UNSIGNED_BYTE;
	case RenderIndexType::UnsignedShort: return GL_UNSIGNED_SHORT;
	case RenderIndexType::UnsignedInt: return GL_UNSIGNED_INT;
	default: return 0;
	}
}

uint32_t ConvertPrimitiveMode(RenderPrimitiveMode mode)
{
	switch (mode)
	{
	case RenderPrimitiveMode::Points: return GL_POINTS;
	case RenderPrimitiveMode::LineStrip: return GL_LINE_STRIP;
	case RenderPrimitiveMode::LineLoop: return GL_LINE_LOOP;
	case RenderPrimitiveMode::Lines: return GL_LINES;
	case RenderPrimitiveMode::TriangleStrip: return GL_TRIANGLE_STRIP;
	case RenderPrimitiveMode::TriangleFan: return GL_TRIANGLE_FAN;
	case RenderPrimitiveMode::Triangles: return GL_TRIANGLES;
	default: return 0;
	}
}

uint32_t ConvertShaderStage(RenderShaderStage stage)
{
	switch (stage)
	{
	case RenderShaderStage::VertexShader: return GL_VERTEX_SHADER;
	case RenderShaderStage::GeometryShader: return GL_GEOMETRY_SHADER;
	case RenderShaderStage::FragmentShader: return GL_FRAGMENT_SHADER;
	case RenderShaderStage::ComputeShader: return GL_COMPUTE_SHADER;
	default: return 0;
	}
}

uint32_t ConvertVertexElemType(RenderVertexElemType type)
{
	switch (type)
	{
	case RenderVertexElemType::Float: return GL_FLOAT;
	default: return 0;
	}
}

RenderDebugSource ConvertDebugSource(uint32_t source)
{
	switch (source)
	{
	case GL_DEBUG_SOURCE_API: return RenderDebugSource::Api;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM: return RenderDebugSource::WindowSystem;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: return RenderDebugSource::ShaderCompiler;
	case GL_DEBUG_SOURCE_THIRD_PARTY: return RenderDebugSource::ThirdParty;
	case GL_DEBUG_SOURCE_APPLICATION: return RenderDebugSource::Application;
	case GL_DEBUG_SOURCE_OTHER: return RenderDebugSource::Other;
	default: return RenderDebugSource::Other;
	}
}

RenderDebugType ConvertDebugType(uint32_t type)
{
	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR: return RenderDebugType::Error;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return RenderDebugType::DeprecatedBehavior;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR: return RenderDebugType::UndefinedBehavior;
	case GL_DEBUG_TYPE_PORTABILITY: return RenderDebugType::Portability;
	case GL_DEBUG_TYPE_PERFORMANCE: return RenderDebugType::Performance;
	case GL_DEBUG_TYPE_MARKER: return RenderDebugType::Marker;
	case GL_DEBUG_TYPE_PUSH_GROUP: return RenderDebugType::PushGroup;
	case GL_DEBUG_TYPE_POP_GROUP: return RenderDebugType::PopGroup;
	case GL_DEBUG_TYPE_OTHER: return RenderDebugType::Other;
	default: return RenderDebugType::Other;
	}
}

RenderDebugSeverity ConvertDebugSeverity(uint32_t severity)
{
	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH: return RenderDebugSeverity::High;
	case GL_DEBUG_SEVERITY_MEDIUM: return RenderDebugSeverity::Medium;
	case GL_DEBUG_SEVERITY_LOW: return RenderDebugSeverity::Low;
	case GL_DEBUG_SEVERITY_NOTIFICATION: return RenderDebugSeverity::Notification;
	default: return RenderDebugSeverity::Notification;
	}
}

uint32_t ConvertObjectType(RenderObjectType type)
{
	switch (type)
	{
	case RenderObjectType::Buffer: return GL_BUFFER;
	case RenderObjectType::Shader: return GL_SHADER;
	case RenderObjectType::Program: return GL_PROGRAM;
	case RenderObjectType::VertexArray: return GL_VERTEX_ARRAY;
	case RenderObjectType::Query: return GL_QUERY;
	case RenderObjectType::ProgramPipeline: return GL_PROGRAM_PIPELINE;
	case RenderObjectType::TransformFeedback: return GL_TRANSFORM_FEEDBACK;
	case RenderObjectType::Sampler: return GL_SAMPLER;
	case RenderObjectType::Texture: return GL_TEXTURE;
	case RenderObjectType::Renderbuffer: return GL_RENDERBUFFER;
	case RenderObjectType::Framebuffer: return GL_FRAMEBUFFER;
	default: return 0;
	}
}