#include "Rendering/RenderDeviceOpenGL.hpp"

#include "System/IncludeOpenGL.hpp"

static unsigned int ConvertDeviceParameter(RenderDeviceParameter parameter)
{
	switch (parameter)
	{
	case RenderDeviceParameter::MaxUniformBlockSize: return GL_MAX_UNIFORM_BLOCK_SIZE;
	case RenderDeviceParameter::UniformBufferOffsetAlignment: return GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT;
	default: return 0;
	}
}

static unsigned int ConvertClipOriginMode(RenderClipOriginMode origin)
{
	switch (origin)
	{
	case RenderClipOriginMode::LowerLeft: return GL_LOWER_LEFT;
	case RenderClipOriginMode::UpperLeft: return GL_UPPER_LEFT;
	default: return 0;
	}
}

static unsigned int ConvertClipDepthMode(RenderClipDepthMode depth)
{
	switch (depth)
	{
	case RenderClipDepthMode::NegativeOneToOne: return GL_NEGATIVE_ONE_TO_ONE;
	case RenderClipDepthMode::ZeroToOne: return GL_ZERO_TO_ONE;
	default: return 0;
	}
}

static unsigned int ConvertBufferUsage(RenderBufferUsage usage)
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

static unsigned int ConvertBufferTarget(RenderBufferTarget target)
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

static unsigned int ConvertBufferAccess(RenderBufferAccess access)
{
	switch (access)
	{
	case RenderBufferAccess::ReadOnly: return GL_READ_ONLY;
	case RenderBufferAccess::WriteOnly: return GL_WRITE_ONLY;
	case RenderBufferAccess::ReadWrite: return GL_READ_WRITE;
	default: return 0;
	}
}

static unsigned int ConvertTextureTarget(RenderTextureTarget target)
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

static unsigned int ConvertTextureDataType(RenderTextureDataType type)
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

static unsigned int ConvertTextureSizedFormat(RenderTextureSizedFormat format)
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

static unsigned int ConvertTextureBaseFormat(RenderTextureBaseFormat format)
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

static unsigned int ConvertTextureParameter(RenderTextureParameter parameter)
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

static unsigned int ConvertTextureFilterMode(RenderTextureFilterMode mode)
{
	switch (mode)
	{
	case RenderTextureFilterMode::Nearest: return GL_NEAREST;
	case RenderTextureFilterMode::Linear: return GL_LINEAR;
	case RenderTextureFilterMode::LinearMipmap: return GL_LINEAR_MIPMAP_LINEAR;
	default: return 0;
	}
}

static unsigned int ConvertTextureWrapMode(RenderTextureWrapMode mode)
{
	switch (mode)
	{
	case RenderTextureWrapMode::Repeat: return GL_REPEAT;
	case RenderTextureWrapMode::MirroredRepeat: return GL_MIRRORED_REPEAT;
	case RenderTextureWrapMode::ClampToEdge: return GL_CLAMP_TO_EDGE;
	default: return 0;
	}
}

static unsigned int ConvertTextureCompareMode(RenderTextureCompareMode mode)
{
	switch (mode)
	{
	case RenderTextureCompareMode::None: return GL_NONE;
	case RenderTextureCompareMode::CompareRefToTexture: return GL_COMPARE_REF_TO_TEXTURE;
	default: return 0;
	}
}

static unsigned int ConvertDepthCompareFunc(RenderDepthCompareFunc func)
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

static unsigned int ConvertFramebufferTarget(RenderFramebufferTarget target)
{
	switch (target)
	{
	case RenderFramebufferTarget::Framebuffer: return GL_FRAMEBUFFER;
	default: return 0;
	}
}

static unsigned int ConvertFramebufferAttachment(RenderFramebufferAttachment attachment)
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

static unsigned int ConvertBlendFactor(RenderBlendFactor factor)
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

static unsigned int ConvertIndexType(RenderIndexType type)
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

static unsigned int ConvertPrimitiveMode(RenderPrimitiveMode mode)
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

static unsigned int ConvertShaderStage(RenderShaderStage stage)
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

static unsigned int ConvertVertexElemType(RenderVertexElemType type)
{
	switch (type)
	{
	case RenderVertexElemType::Float: return GL_FLOAT;
	default: return 0;
	}
}

static RenderDebugSource ConvertDebugSource(GLenum source)
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

static RenderDebugType ConvertDebugType(GLenum type)
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

static RenderDebugSeverity ConvertDebugSeverity(GLenum severity)
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

static unsigned int ConvertObjectType(RenderObjectType type)
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
		StringRef(msg, static_cast<unsigned int>(length))
	};

	if (data->callback)
		data->callback(message);
}

RenderDeviceOpenGL::RenderDeviceOpenGL() :
	debugUserData{ nullptr }
{
}

void RenderDeviceOpenGL::SetDebugMessageCallback(DebugCallbackFn callback)
{
	debugUserData.callback = callback;
	glDebugMessageCallback(DebugMessageCallback, &debugUserData);
}

void RenderDeviceOpenGL::SetObjectLabel(RenderObjectType type, unsigned int object, StringRef label)
{
	glObjectLabel(ConvertObjectType(type), object, label.len, label.str);
}

void RenderDeviceOpenGL::SetObjectPtrLabel(void* ptr, StringRef label)
{
	glObjectPtrLabel(ptr, label.len, label.str);
}

void RenderDeviceOpenGL::GetIntegerValue(RenderDeviceParameter parameter, int* valueOut)
{
	glGetIntegerv(ConvertDeviceParameter(parameter), valueOut);
}

void RenderDeviceOpenGL::PushDebugGroup(unsigned int id, StringRef message)
{
	glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, id, message.len, message.str);
}

void RenderDeviceOpenGL::PopDebugGroup()
{
	glPopDebugGroup();
}

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

void RenderDeviceOpenGL::CreateFramebuffers(unsigned int count, unsigned int* framebuffersOut)
{
	glGenFramebuffers(count, framebuffersOut);
}

void RenderDeviceOpenGL::DestroyFramebuffers(unsigned int count, unsigned int* framebuffers)
{
	glDeleteFramebuffers(count, framebuffers);
}

void RenderDeviceOpenGL::BindFramebuffer(const RenderCommandData::BindFramebufferData* data)
{
	BindFramebuffer(data->target, data->framebuffer);
}

void RenderDeviceOpenGL::BindFramebuffer(RenderFramebufferTarget target, unsigned int framebuffer)
{
	glBindFramebuffer(ConvertFramebufferTarget(target), framebuffer);
}

void RenderDeviceOpenGL::AttachFramebufferTexture2D(const RenderCommandData::AttachFramebufferTexture2D* data)
{
	glFramebufferTexture2D(ConvertFramebufferTarget(data->target), ConvertFramebufferAttachment(data->attachment),
		ConvertTextureTarget(data->textureTarget), data->texture, data->mipLevel);
}

void RenderDeviceOpenGL::SetFramebufferDrawBuffers(unsigned int count, const RenderFramebufferAttachment* buffers)
{
	unsigned int attachments[16];

	for (unsigned int i = 0; i < count; ++i)
		attachments[i] = ConvertFramebufferAttachment(buffers[i]);

	glDrawBuffers(count, attachments);
}

// TEXTURE

void RenderDeviceOpenGL::CreateTextures(unsigned int count, unsigned int* texturesOut)
{
	glGenTextures(count, texturesOut);
}

void RenderDeviceOpenGL::DestroyTextures(unsigned int count, unsigned int* textures)
{
	glDeleteTextures(count, textures);
}

void RenderDeviceOpenGL::BindTexture(RenderTextureTarget target, unsigned int texture)
{
	glBindTexture(ConvertTextureTarget(target), texture);
}

void RenderDeviceOpenGL::SetTextureStorage2D(const RenderCommandData::SetTextureStorage2D* data)
{
	glTexStorage2D(ConvertTextureTarget(data->target), data->levels,
		ConvertTextureSizedFormat(data->format), data->width, data->height);
}

void RenderDeviceOpenGL::SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data)
{
	glTexImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->internalFormat,
		data->width, data->height, 0, data->format, data->type, data->data);
}

void RenderDeviceOpenGL::SetTextureSubImage2D(const RenderCommandData::SetTextureSubImage2D* data)
{
	glTexSubImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->xOffset, data->yOffset,
		data->width, data->height, ConvertTextureBaseFormat(data->format),
		ConvertTextureDataType(data->type), data->data);
}

void RenderDeviceOpenGL::SetTextureImageCompressed2D(const RenderCommandData::SetTextureImageCompressed2D* data)
{
	glCompressedTexImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->internalFormat,
		data->width, data->height, 0, data->dataSize, data->data);
}

void RenderDeviceOpenGL::GenerateTextureMipmaps(RenderTextureTarget target)
{
	glGenerateMipmap(ConvertTextureTarget(target));
}

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

void RenderDeviceOpenGL::CreateSamplers(unsigned int count, unsigned int* samplersOut)
{
	glGenSamplers(count, samplersOut);
}

void RenderDeviceOpenGL::DestroySamplers(unsigned int count, unsigned int* samplers)
{
	glDeleteSamplers(count, samplers);
}

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

void RenderDeviceOpenGL::UseShaderProgram(unsigned int shaderProgram)
{
	glUseProgram(shaderProgram);
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

// VERTEX ARRAY

void RenderDeviceOpenGL::CreateVertexArrays(unsigned int count, unsigned int* vertexArraysOut)
{
	glGenVertexArrays(count, vertexArraysOut);
}

void RenderDeviceOpenGL::DestroyVertexArrays(unsigned int count, unsigned int* vertexArrays)
{
	glDeleteVertexArrays(count, vertexArrays);
}

void RenderDeviceOpenGL::BindVertexArray(unsigned int vertexArrayId)
{
	glBindVertexArray(vertexArrayId);
}

void RenderDeviceOpenGL::EnableVertexAttribute(unsigned int index)
{
	glEnableVertexAttribArray(index);
}

void RenderDeviceOpenGL::SetVertexAttributePointer(const RenderCommandData::SetVertexAttributePointer* data)
{
	glVertexAttribPointer(data->attributeIndex, data->elementCount, ConvertVertexElemType(data->elementType),
		GL_FALSE, data->stride, reinterpret_cast<void*>(data->offset));
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

void RenderDeviceOpenGL::CreateBuffers(unsigned int count, unsigned int* buffersOut)
{
	glGenBuffers(count, buffersOut);
}

void RenderDeviceOpenGL::DestroyBuffers(unsigned int count, unsigned int* buffers)
{
	glDeleteBuffers(count, buffers);
}

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

void RenderDeviceOpenGL::SetBufferStorage(const RenderCommandData::SetBufferStorage* data)
{
	GLbitfield bits = 0;
	if (data->dynamicStorage) bits |= GL_DYNAMIC_STORAGE_BIT;
	if (data->mapReadAccess) bits |= GL_MAP_READ_BIT;
	if (data->mapWriteAccess) bits |= GL_MAP_WRITE_BIT;
	if (data->mapPersistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (data->mapCoherent) bits |= GL_MAP_COHERENT_BIT;

	glBufferStorage(ConvertBufferTarget(data->target), data->size, data->data, bits);
}

void RenderDeviceOpenGL::SetBufferData(RenderBufferTarget target, unsigned int size, const void* data, RenderBufferUsage usage)
{
	glBufferData(ConvertBufferTarget(target), size, data, ConvertBufferUsage(usage));
}

void RenderDeviceOpenGL::SetBufferSubData(RenderBufferTarget target, unsigned int offset, unsigned int size, const void* data)
{
	glBufferSubData(ConvertBufferTarget(target), offset, size, data);
}

void* RenderDeviceOpenGL::MapBuffer(RenderBufferTarget target, RenderBufferAccess access)
{
	return glMapBuffer(ConvertBufferTarget(target), ConvertBufferAccess(access));
}

void* RenderDeviceOpenGL::MapBufferRange(const RenderCommandData::MapBufferRange* data)
{
	GLbitfield bits = 0;
	if (data->readAccess) bits |= GL_MAP_READ_BIT;
	if (data->writeAccess) bits |= GL_MAP_WRITE_BIT;
	if (data->invalidateRange) bits |= GL_MAP_INVALIDATE_RANGE_BIT;
	if (data->invalidateBuffer) bits |= GL_MAP_INVALIDATE_BUFFER_BIT;
	if (data->flushExplicit) bits |= GL_MAP_FLUSH_EXPLICIT_BIT;
	if (data->unsynchronized) bits |= GL_MAP_UNSYNCHRONIZED_BIT;
	if (data->persistent) bits |= GL_MAP_PERSISTENT_BIT;
	if (data->coherent) bits |= GL_MAP_COHERENT_BIT;

	return glMapBufferRange(ConvertBufferTarget(data->target), data->offset, data->length, bits);
}

void RenderDeviceOpenGL::UnmapBuffer(RenderBufferTarget target)
{
	glUnmapBuffer(ConvertBufferTarget(target));
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
