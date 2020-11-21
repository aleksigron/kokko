#pragma once

#include <cstdint>

#include "Core/StringRef.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

enum class UniformDataType
{
	Tex2D,
	TexCube,
	Mat4x4,
	Vec4,
	Vec3,
	Vec2,
	Float,
	Int
};

struct ShaderUniform
{
	static const unsigned int MaxBufferUniformCount = 8;
	static const unsigned int MaxTextureUniformCount = 8;

	StringRef name;
	uint32_t nameHash;
	UniformDataType type;
};

struct BufferUniform : ShaderUniform
{
	unsigned int dataOffset;
	unsigned int bufferObjectOffset;

	template <typename ValueType>
	const ValueType& GetValue(unsigned char* dataBuffer)
	{
		return *reinterpret_cast<ValueType*>(dataBuffer + dataOffset);
	}

	void SetValueInt(unsigned char* dataBuffer, int value) const
	{
		*reinterpret_cast<int*>(dataBuffer + dataOffset) = value;
	}

	void SetValueFloat(unsigned char* dataBuffer, float value) const
	{
		*reinterpret_cast<float*>(dataBuffer + dataOffset) = value;
	}

	void SetValueFloatVec(unsigned char* dataBuffer, const float* values, unsigned int count) const
	{
		for (unsigned int i = 0; i < count; ++i)
			*reinterpret_cast<float*>(dataBuffer + dataOffset + i * sizeof(float)) = values[i];
	}

	void UpdateToUniformBuffer(unsigned char* dataBuffer, unsigned char* uniformBuffer);
};

struct TextureUniform : ShaderUniform
{
	int uniformLocation;
	RenderTextureTarget textureTarget;
	unsigned int textureName;
};

struct UniformTypeInfo
{
	unsigned int size;
	unsigned int alignment;

	unsigned int typeNameLength;
	const char* typeName;

	bool isTexture;

	static const unsigned int TypeCount = 8;
	static const UniformTypeInfo Types[TypeCount];

	static const UniformTypeInfo& FromType(UniformDataType type)
	{
		return UniformTypeInfo::Types[static_cast<size_t>(type)];
	}
};