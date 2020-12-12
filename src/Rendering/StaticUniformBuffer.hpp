#pragma once

#include <cstddef>

#include "Rendering/UniformBuffer.hpp"

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
private:
	static constexpr size_t MaxLightCount = 8;
	static constexpr size_t MaxCascadeCount = 4;

	static constexpr size_t LightArraySize = MaxLightCount * 16;
	static constexpr size_t ShadowMatOffset = LightArraySize * 3;
	static constexpr size_t ShadowSplitOffset = ShadowMatOffset + MaxCascadeCount * 64;
	static constexpr size_t PerspectiveMatOffset = ShadowSplitOffset + (MaxCascadeCount + 1) * 16;
	static constexpr size_t LightCountOffset = PerspectiveMatOffset + 64 + 16 + 8;
	static constexpr size_t ShadowBiasOffset = LightCountOffset + 3 * 4;

public:
	static const std::size_t BufferSize = ShadowBiasOffset + 3 * 4;
	static const unsigned int BindingPoint = 0;

	static UniformBlockArray<Vec3f, LightArraySize * 0> lightColors;
	static UniformBlockArray<Vec3f, LightArraySize * 1> lightPositions;
	static UniformBlockArray<Vec4f, LightArraySize * 2> lightDirections;

	static UniformBlockArray<Mat4x4f, ShadowMatOffset> shadowMatrices;
	static UniformBlockArray<float, ShadowSplitOffset> shadowSplits;

	static UniformBlockScalar<Mat4x4f, PerspectiveMatOffset> perspectiveMatrix;
	static UniformBlockScalar<Vec3f, PerspectiveMatOffset + 64> ambientColor;
	static UniformBlockScalar<Vec2f, PerspectiveMatOffset + 64 + 16> halfNearPlane;

	static UniformBlockScalar<int, LightCountOffset + 0> pointLightCount;
	static UniformBlockScalar<int, LightCountOffset + 4> spotLightCount;
	static UniformBlockScalar<int, LightCountOffset + 8> cascadeCount;

	static UniformBlockScalar<float, ShadowBiasOffset + 0> shadowBiasOffset;
	static UniformBlockScalar<float, ShadowBiasOffset + 4> shadowBiasFactor;
	static UniformBlockScalar<float, ShadowBiasOffset + 8> shadowBiasClamp;
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
