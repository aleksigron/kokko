#pragma once

#include <cstdint>

#include "Core/StringRef.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "Rendering/RenderDeviceEnums.hpp"

#include "Resources/TextureId.hpp"

enum class UniformDataType
{
	Tex2D,
	TexCube,
	Mat4x4,
	Mat4x4Array,
	Mat3x3,
	Mat3x3Array,
	Vec4,
	Vec4Array,
	Vec3,
	Vec3Array,
	Vec2,
	Vec2Array,
	Float,
	FloatArray,
	Int,
	IntArray
};

struct ShaderUniform
{
	StringRef name;
	uint32_t nameHash;
	UniformDataType type;
};

struct BufferUniform : ShaderUniform
{
	unsigned int dataOffset;
	unsigned int bufferObjectOffset;
	unsigned char arraySize; // If type is not an array type, arraySize is 0

	template <typename ValueType>
	const ValueType* GetArray(unsigned char* dataBuffer, unsigned int& countOut)
	{
		countOut = *reinterpret_cast<unsigned int*>(dataBuffer + dataOffset);
		return reinterpret_cast<ValueType*>(dataBuffer + dataOffset + sizeof(unsigned int));
	}

	template <typename ValueType>
	const ValueType& GetValue(unsigned char* dataBuffer)
	{
		return *reinterpret_cast<ValueType*>(dataBuffer + dataOffset);
	}

	void SetValueInt(unsigned char* dataBuffer, int value) const;
	void SetArrayInt(unsigned char* dataBuffer, const int* values, unsigned int count) const;

	void SetValueFloat(unsigned char* dataBuffer, float value) const;
	void SetArrayFloat(unsigned char* dataBuffer, const float* values, unsigned int count) const;

	void SetValueFloatVec(unsigned char* dataBuffer, unsigned int count, const float* values) const;

	void SetValueVec2f(unsigned char* dataBuffer, const Vec2f& value) const;
	void SetArrayVec2f(unsigned char* dataBuffer, const Vec2f* values, unsigned int count) const;

	void SetValueVec3f(unsigned char* dataBuffer, const Vec3f& value) const;
	void SetArrayVec3f(unsigned char* dataBuffer, const Vec3f* values, unsigned int count) const;

	void SetValueVec4f(unsigned char* dataBuffer, const Vec4f& value) const;
	void SetArrayVec4f(unsigned char* dataBuffer, const Vec4f* values, unsigned int count) const;

	void SetValueMat3x3f(unsigned char* dataBuffer, const Mat3x3f& value) const;
	void SetArrayMat3x3f(unsigned char* dataBuffer, const Mat3x3f* values, unsigned int count) const;

	void SetValueMat4x4f(unsigned char* dataBuffer, const Mat4x4f& value) const;
	void SetArrayMat4x4f(unsigned char* dataBuffer, const Mat4x4f* values, unsigned int count) const;

	void UpdateToUniformBuffer(unsigned char* dataBuffer, unsigned char* uniformBuffer);
};

struct TextureUniform : ShaderUniform
{
	int uniformLocation;
	RenderTextureTarget textureTarget;
	TextureId textureId;
	unsigned int textureObject;
};

struct UniformList
{
	unsigned int uniformDataSize; // CPU side
	unsigned int uniformBufferSize; // GPU side

	unsigned int bufferUniformCount;
	unsigned int textureUniformCount;
	BufferUniform* bufferUniforms;
	TextureUniform* textureUniforms;

	UniformList();

	BufferUniform* FindBufferUniformByName(StringRef name);
	const BufferUniform* FindBufferUniformByName(StringRef name) const;

	BufferUniform* FindBufferUniformByNameHash(uint32_t nameHash);
	const BufferUniform* FindBufferUniformByNameHash(uint32_t nameHash) const;

	TextureUniform* FindTextureUniformByName(StringRef name);
	const TextureUniform* FindTextureUniformByName(StringRef name) const;

	TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash);
	const TextureUniform* FindTextureUniformByNameHash(uint32_t nameHash) const;
};

struct UniformTypeInfo
{
	unsigned int size;
	unsigned int alignment;

	unsigned int typeNameLength;
	const char* typeName;

	bool isArray;
	bool isTexture;

	static const unsigned int TypeCount = 16;
	static const UniformTypeInfo Types[TypeCount];

	UniformTypeInfo(unsigned int size, unsigned int alignment,
		const char* typeName, bool isArray, bool isTexture);

	static const UniformTypeInfo& FromType(UniformDataType type)
	{
		return UniformTypeInfo::Types[static_cast<size_t>(type)];
	}
};