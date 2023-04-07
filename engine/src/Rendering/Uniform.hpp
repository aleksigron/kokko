#pragma once

#include <cstddef>
#include <cstdint>

#include "Core/StringView.hpp"

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Math/Mat3x3.hpp"
#include "Math/Mat4x4.hpp"

#include "Rendering/RenderTypes.hpp"
#include "Rendering/RenderResourceId.hpp"

#include "Resources/TextureId.hpp"

namespace kokko
{

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
	ConstStringView name;
	uint32_t nameHash;
	UniformDataType type;
};

struct BufferUniform : ShaderUniform
{
	unsigned int dataOffset;
	unsigned int bufferObjectOffset;
	unsigned int arraySize; // If type is not an array type, arraySize is 0
};

struct TextureUniform : ShaderUniform
{
	int uniformLocation;
	TextureId textureId;
	kokko::RenderTextureId textureObject;
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

}
