#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Vec4.hpp"
#include "Math/Mat4x4.hpp"

namespace UniformBuffer
{
	inline void SetScalarFloat(unsigned char* buffer, size_t offset, float value)
	{
		*reinterpret_cast<float*>(buffer + offset) = value;
	}

	inline void SetScalarInt(unsigned char* buffer, size_t offset, int value)
	{
		*reinterpret_cast<int*>(buffer + offset) = value;
	}

	inline void SetScalarVec2f(unsigned char* buffer, size_t offset, const Vec2f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + 4) = value.y;
	}

	inline void SetScalarVec3f(unsigned char* buffer, size_t offset, const Vec3f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + 4) = value.y;
		*reinterpret_cast<float*>(buffer + offset + 8) = value.z;
	}

	inline void SetScalarVec4f(unsigned char* buffer, size_t offset, const Vec4f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + 4) = value.y;
		*reinterpret_cast<float*>(buffer + offset + 8) = value.z;
		*reinterpret_cast<float*>(buffer + offset + 12) = value.w;
	}

	inline void SetScalarMat3x3f(unsigned char* buffer, size_t offset, const Mat3x3f& value)
	{
		for (size_t i = 0; i < 3; ++i)
			for (size_t j = 0; j < 3; ++j)
				*reinterpret_cast<float*>(buffer + offset + i * 16 + j * 4) = value[i * 3 + j];
	}

	inline void SetScalarMat4x4f(unsigned char* buffer, size_t offset, const Mat4x4f& value)
	{
		for (size_t i = 0; i < 16; ++i)
			*reinterpret_cast<float*>(buffer + offset + i * 4) = value[i];
	}

	inline void SetArrayFloatOne(unsigned char* buffer, size_t offset, size_t index, float value)
	{
		*reinterpret_cast<float*>(buffer + offset + index * 16) = value;
	}
	inline void SetArrayFloatMany(unsigned char* buffer, size_t offset, size_t count, const float* values)
	{
		for (size_t i = 0; i < count; ++i)
			*reinterpret_cast<float*>(buffer + offset + i * 16) = values[i];
	}

	inline void SetArrayIntOne(unsigned char* buffer, size_t offset, size_t index, int value)
	{
		*reinterpret_cast<int*>(buffer + offset + index * 16) = value;
	}
	inline void SetArrayIntMany(unsigned char* buffer, size_t offset, size_t count, const int* values)
	{
		for (size_t i = 0; i < count; ++i)
			*reinterpret_cast<int*>(buffer + offset + i * 16) = values[i];
	}

	inline void SetArrayVec2fOne(unsigned char* buffer, size_t offset, size_t index, const Vec2f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 4) = value.y;
	}
	inline void SetArrayVec2fMany(unsigned char* buffer, size_t offset, size_t count, const Vec2f* values)
	{
		for (size_t i = 0; i < count; ++i)
		{
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 0) = values[i].x;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 4) = values[i].y;
		}
	}

	inline void SetArrayVec3fOne(unsigned char* buffer, size_t offset, size_t index, const Vec3f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 4) = value.y;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 8) = value.z;
	}
	inline void SetArrayVec3fMany(unsigned char* buffer, size_t offset, size_t count, const Vec3f* values)
	{
		for (size_t i = 0; i < count; ++i)
		{
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 0) = values[i].x;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 4) = values[i].y;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 8) = values[i].z;
		}
	}

	inline void SetArrayVec4fOne(unsigned char* buffer, size_t offset, size_t index, const Vec4f& value)
	{
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 0) = value.x;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 4) = value.y;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 8) = value.z;
		*reinterpret_cast<float*>(buffer + offset + index * 16 + 12) = value.w;
	}
	inline void SetArrayVec4fMany(unsigned char* buffer, size_t offset, size_t count, const Vec4f* values)
	{
		for (size_t i = 0; i < count; ++i)
		{
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 0) = values[i].x;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 4) = values[i].y;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 8) = values[i].z;
			*reinterpret_cast<float*>(buffer + offset + i * 16 + 12) = values[i].w;
		}
	}

	inline void SetArrayMat3x3fOne(unsigned char* buffer, size_t offset, size_t index, const Mat3x3f& value)
	{
		for (size_t j = 0; j < 3; ++j)
			for (size_t k = 0; k < 3; ++k)
				*reinterpret_cast<float*>(buffer + offset + index * 64 + j * 16 + k) = value[j * 3 + k];
	}
	inline void SetArrayMat3x3fMany(unsigned char* buffer, size_t offset, size_t count, const Mat3x3f* values)
	{
		for (size_t i = 0; i < count; ++i)
			for (size_t j = 0; j < 3; ++j)
				for (size_t k = 0; k < 3; ++k)
					*reinterpret_cast<float*>(buffer + offset + i * 64 + j * 16 + k) = values[i][j * 3 + k];
	}

	inline void SetArrayMat4x4fOne(unsigned char* buffer, size_t offset, size_t index, const Mat4x4f& value)
	{
		for (size_t i = 0; i < 16; ++i)
			*reinterpret_cast<float*>(buffer + offset + index * 64 + i * 4) = value[i];
	}
	inline void SetArrayMat4x4fMany(unsigned char* buffer, size_t offset, size_t count, const Mat4x4f* values)
	{
		for (size_t i = 0; i < count; ++i)
			for (size_t j = 0; j < 16; ++j)
				*reinterpret_cast<float*>(buffer + offset + i * 64 + j * 4) = values[i][j];
	}
}
