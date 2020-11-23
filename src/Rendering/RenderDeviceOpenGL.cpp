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

void RenderDeviceOpenGL::SetTextureParameterInt(RenderTextureTarget target, unsigned int parameter, unsigned int value)
{
	glTexParameteri(ConvertTextureTarget(target), parameter, value);
}

void RenderDeviceOpenGL::SetActiveTextureUnit(unsigned int textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
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
	glVertexAttribPointer(data->attributeIndex, data->elementCount, data->elementType,
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
