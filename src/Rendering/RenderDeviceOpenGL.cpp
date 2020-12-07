#include "Rendering/RenderDeviceOpenGL.hpp"

#include "System/IncludeOpenGL.hpp"

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

static unsigned int ConvertTextureCompareFunc(RenderTextureCompareFunc func)
{
	switch (func)
	{
	case RenderTextureCompareFunc::LessThanOrEqual: return GL_LEQUAL;
	case RenderTextureCompareFunc::GreaterThanOrEqual: return GL_GEQUAL;
	case RenderTextureCompareFunc::Less: return GL_LESS;
	case RenderTextureCompareFunc::Greater: return GL_GREATER;
	case RenderTextureCompareFunc::Equal: return GL_EQUAL;
	case RenderTextureCompareFunc::NotEqual: return GL_NOTEQUAL;
	case RenderTextureCompareFunc::Always: return GL_ALWAYS;
	case RenderTextureCompareFunc::Never: return GL_NEVER;
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

void RenderDeviceOpenGL::Clear(unsigned int mask)
{
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

void RenderDeviceOpenGL::DepthRange(const RenderCommandData::DepthRangeData* data)
{
	glDepthRange(data->near, data->far);
}

void RenderDeviceOpenGL::Viewport(const RenderCommandData::ViewportData* data)
{
	glViewport(data->x, data->y, data->w, data->h);
}

// DEPTH TEST / WRITE

void RenderDeviceOpenGL::DepthTestEnable()
{
	glEnable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestDisable()
{
	glDisable(GL_DEPTH_TEST);
}

void RenderDeviceOpenGL::DepthTestFunction(unsigned int function)
{
	glDepthFunc(function);
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
	glFramebufferTexture2D(ConvertFramebufferTarget(data->target), data->attachment,
		ConvertTextureTarget(data->textureTarget), data->texture, data->mipLevel);
}

void RenderDeviceOpenGL::SetFramebufferDrawBuffers(unsigned int count, unsigned int* buffers)
{
	glDrawBuffers(count, buffers);
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

void RenderDeviceOpenGL::SetTextureImage2D(const RenderCommandData::SetTextureImage2D* data)
{
	glTexImage2D(ConvertTextureTarget(data->target), data->mipLevel, data->internalFormat,
		data->width, data->height, 0, data->format, data->type, data->data);
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

void RenderDeviceOpenGL::SetTextureCompareFunc(RenderTextureTarget target, RenderTextureCompareFunc func)
{
	SetTextureParameterInt(target, RenderTextureParameter::CompareFunc, ConvertTextureCompareFunc(func));
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

void RenderDeviceOpenGL::BindVertexArray(unsigned int vertexArray)
{
	glBindVertexArray(vertexArray);
}

void RenderDeviceOpenGL::DrawIndexed(RenderPrimitiveMode mode, int indexCount, RenderIndexType indexType)
{
	glDrawElements(ConvertPrimitiveMode(mode), indexCount, ConvertIndexType(indexType), nullptr);
}

void RenderDeviceOpenGL::Draw(RenderPrimitiveMode mode, int offset, int vertexCount)
{
	glDrawArrays(ConvertPrimitiveMode(mode), offset, vertexCount);
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

void RenderDeviceOpenGL::SetBufferData(RenderBufferTarget target, unsigned int size, const void* data, RenderBufferUsage usage)
{
	glBufferData(ConvertBufferTarget(target), size, data, ConvertBufferUsage(usage));
}

void RenderDeviceOpenGL::SetBufferSubData(RenderBufferTarget target, unsigned int offset, unsigned int size, const void* data)
{
	glBufferSubData(ConvertBufferTarget(target), offset, size, data);
}
