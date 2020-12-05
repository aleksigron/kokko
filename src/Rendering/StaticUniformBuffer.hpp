#pragma once

#include <cstddef>

#include "Rendering/UniformBufferData.hpp"

template <typename ValueType, size_t Offset>
struct UniformBlockScalar
{
	static void Set(unsigned char* buffer, const ValueType& value)
	{
		*reinterpret_cast<ValueType*>(buffer + Offset) = value;
	}
};

template <size_t Offset>
struct UniformBlockScalar<Vec2f, Offset>
{
	static void Set(unsigned char* buffer, const Vec2f& value)
	{
		UniformBuffer::SetScalarVec2f(buffer, Offset, value);
	}
};

template <size_t Offset>
struct UniformBlockScalar<Vec3f, Offset>
{
	static void Set(unsigned char* buffer, const Vec3f& value)
	{
		UniformBuffer::SetScalarVec3f(buffer, Offset, value);
	}
};

template <size_t Offset>
struct UniformBlockScalar<Vec4f, Offset>
{
	static void Set(unsigned char* buffer, const Vec4f& value)
	{
		UniformBuffer::SetScalarVec4f(buffer, Offset, value);
	}
};

template <size_t Offset>
struct UniformBlockScalar<Mat4x4f, Offset>
{
	static void Set(unsigned char* buffer, const Mat4x4f& value)
	{
		UniformBuffer::SetScalarMat4x4f(buffer, Offset, value);
	}
};

template <typename ValueType, size_t Offset>
class UniformBlockArray
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const ValueType& value)
	{
		*reinterpret_cast<ValueType*>(buffer + Offset + index * 16) = value;
	}

	static void SetMany(unsigned char* buffer, size_t count, const ValueType* values)
	{
		for (size_t i = 0; i < count; ++i)
			*reinterpret_cast<ValueType*>(buffer + Offset + i * 16) = values[i];
	}
};

template <size_t Offset>
class UniformBlockArray<Vec2f, Offset>
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const Vec2f& value)
	{
		UniformBuffer::SetArrayVec2fOne(buffer, Offset, index, value);
	}

	static void SetMany(unsigned char* buffer, size_t count, const Vec2f* values)
	{
		UniformBuffer::SetArrayVec2fMany(buffer, Offset, count, values);
	}
};

template <size_t Offset>
class UniformBlockArray<Vec3f, Offset>
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const Vec3f& value)
	{
		UniformBuffer::SetArrayVec3fOne(buffer, Offset, index, value);
	}

	static void SetMany(unsigned char* buffer, size_t count, const Vec3f* values)
	{
		UniformBuffer::SetArrayVec3fMany(buffer, Offset, count, values);
	}
};

template <size_t Offset>
class UniformBlockArray<Vec4f, Offset>
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const Vec4f& value)
	{
		UniformBuffer::SetArrayVec4fOne(buffer, Offset, index, value);
	}

	static void SetMany(unsigned char* buffer, size_t count, const Vec4f* values)
	{
		UniformBuffer::SetArrayVec4fMany(buffer, Offset, count, values);
	}
};

template <size_t Offset>
class UniformBlockArray<Mat3x3f, Offset>
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const Mat3x3f& value)
	{
		UniformBuffer::SetArrayMat3x3fOne(buffer, Offset, index, value);
	}

	static void SetMany(unsigned char* buffer, size_t count, const Mat3x3f* values)
	{
		UniformBuffer::SetArrayMat3x3fMany(buffer, Offset, count, values);
	}
};

template <size_t Offset>
class UniformBlockArray<Mat4x4f, Offset>
{
public:
	static void SetOne(unsigned char* buffer, size_t index, const Mat4x4f& value)
	{
		UniformBuffer::SetArrayMat4x4fOne(buffer, Offset, index, value);
	}

	static void SetMany(unsigned char* buffer, size_t count, const Mat4x4f* values)
	{
		UniformBuffer::SetArrayMat4x4fMany(buffer, Offset, count, values);
	}
};

class LightingUniformBlock
{
public:
	static const std::size_t BufferSize = 944;
	static const unsigned int BindingPoint = 0;

	static UniformBlockScalar<int, 0> pointLightCount;
	static UniformBlockScalar<int, 4> spotLightCount;
	static UniformBlockScalar<int, 8> cascadeCount;
	static UniformBlockScalar<Vec2f, 16> halfNearPlane;
	static UniformBlockScalar<Mat4x4f, 32> perspectiveMatrix;
	static UniformBlockArray<Vec3f, 96> lightColors;
	static UniformBlockArray<Vec3f, 224> lightPositions;
	static UniformBlockArray<Vec3f, 352> lightDirections;
	static UniformBlockArray<float, 480> lightAngles;
	static UniformBlockArray<Mat4x4f, 608> shadowMatrices;
	static UniformBlockArray<float, 864> shadowSplits;
};

struct ViewportUniformBlock
{
	static const std::size_t BufferSize = 192;
	static const unsigned int BindingPoint = 1;

	static UniformBlockScalar<Mat4x4f, 0> VP;
	static UniformBlockScalar<Mat4x4f, 64> V;
	static UniformBlockScalar<Mat4x4f, 128> P;
};

struct TransformUniformBlock
{
	static const std::size_t BufferSize = 192;
	static const unsigned int BindingPoint = 2;

	static UniformBlockScalar<Mat4x4f, 0> MVP;
	static UniformBlockScalar<Mat4x4f, 64> MV;
	static UniformBlockScalar<Mat4x4f, 128> M;
};

struct MaterialUniformBlock
{
	static const unsigned int BindingPoint = 3;
};
