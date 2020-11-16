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

	inline void SetScalarMat4x4f(unsigned char* buffer, size_t offset, const Mat4x4f& value)
	{
		for (size_t i = 0; i < 16; ++i)
		{
			*reinterpret_cast<float*>(buffer + offset + i * 4) = value[i];
		}
	}

	template <typename ValueType, size_t Offset>
	struct ScalarUniform
	{
		static void Set(unsigned char* buffer, const ValueType& value)
		{
			*reinterpret_cast<ValueType*>(buffer + Offset) = value;
		}
	};

	template <size_t Offset>
	struct ScalarUniform<Vec2f, Offset>
	{
		static void Set(unsigned char* buffer, const Vec2f& value)
		{
			SetScalarVec2f(buffer, Offset, value);
		}
	};

	template <size_t Offset>
	struct ScalarUniform<Vec3f, Offset>
	{
		static void Set(unsigned char* buffer, const Vec3f& value)
		{
			SetScalarVec3f(buffer, Offset, value);
		}
	};

	template <size_t Offset>
	struct ScalarUniform<Vec4f, Offset>
	{
		static void Set(unsigned char* buffer, const Vec4f& value)
		{
			SetScalarVec4f(buffer, Offset, value);
		}
	};

	template <size_t Offset>
	struct ScalarUniform<Mat4x4f, Offset>
	{
		static void Set(unsigned char* buffer, const Mat4x4f& value)
		{
			SetScalarMat4x4f(buffer, Offset, value);
		}
	};

	template <typename ValueType, size_t Offset>
	class ArrayUniform
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
	class ArrayUniform<Vec2f, Offset>
	{
	public:
		static void SetOne(unsigned char* buffer, size_t index, const Vec2f& value)
		{
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 0) = value.x;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 4) = value.y;
		}

		static void SetMany(unsigned char* buffer, size_t count, const Vec2f* values)
		{
			for (size_t i = 0; i < count; ++i)
			{
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 0) = values[i].x;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 4) = values[i].y;
			}
		}
	};

	template <size_t Offset>
	class ArrayUniform<Vec3f, Offset>
	{
	public:
		static void SetOne(unsigned char* buffer, size_t index, const Vec3f& value)
		{
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 0) = value.x;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 4) = value.y;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 8) = value.z;
		}

		static void SetMany(unsigned char* buffer, size_t count, const Vec3f* values)
		{
			for (size_t i = 0; i < count; ++i)
			{
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 0) = values[i].x;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 4) = values[i].y;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 8) = values[i].z;
			}
		}
	};

	template <size_t Offset>
	class ArrayUniform<Vec4f, Offset>
	{
	public:
		static void SetOne(unsigned char* buffer, size_t index, const Vec4f& value)
		{
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 0) = value.x;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 4) = value.y;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 8) = value.z;
			*reinterpret_cast<float*>(buffer + Offset + index * 16 + 12) = value.w;
		}

		static void SetMany(unsigned char* buffer, size_t count, const Vec4f* values)
		{
			for (size_t i = 0; i < count; ++i)
			{
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 0) = values[i].x;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 4) = values[i].y;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 8) = values[i].z;
				*reinterpret_cast<float*>(buffer + Offset + i * 16 + 12) = values[i].w;
			}
		}
	};

	template <size_t Offset>
	class ArrayUniform<Mat4x4f, Offset>
	{
	public:
		static void SetOne(unsigned char* buffer, size_t index, const Mat4x4f& value)
		{
			for (size_t i = 0; i < 16; ++i)
				*reinterpret_cast<float*>(buffer + Offset + index * 64 + i * 4) = value[i];
		}

		static void SetMany(unsigned char* buffer, size_t count, const Mat4x4f* values)
		{
			for (size_t i = 0; i < count; ++i)
				for (size_t j = 0; j < 16; ++j)
					*reinterpret_cast<float*>(buffer + Offset + i * 64 + j * 4) = values[i][j];
		}
	};

	class LightingBlock
	{
	public:
		static const std::size_t BufferSize = 944;
		static const unsigned int BindingPoint = 0;

		static ScalarUniform<int, 0> pointLightCount;
		static ScalarUniform<int, 4> spotLightCount;
		static ScalarUniform<int, 8> cascadeCount;
		static ScalarUniform<Vec2f, 16> halfNearPlane;
		static ScalarUniform<Mat4x4f, 32> perspectiveMatrix;
		static ArrayUniform<Vec3f, 96> lightColors;
		static ArrayUniform<Vec3f, 224> lightPositions;
		static ArrayUniform<Vec3f, 352> lightDirections;
		static ArrayUniform<float, 480> lightAngles;
		static ArrayUniform<Mat4x4f, 608> shadowMatrices;
		static ArrayUniform<float, 864> shadowSplits;
	};

	struct ViewportBlock
	{
		static const std::size_t BufferSize = 192;
		static const unsigned int BindingPoint = 1;

		static ScalarUniform<Mat4x4f, 0> VP;
		static ScalarUniform<Mat4x4f, 64> V;
		static ScalarUniform<Mat4x4f, 128> P;
	};

	struct TransformBlock
	{
		static const std::size_t BufferSize = 192;
		static const unsigned int BindingPoint = 2;

		static ScalarUniform<Mat4x4f, 0> MVP;
		static ScalarUniform<Mat4x4f, 64> MV;
		static ScalarUniform<Mat4x4f, 128> M;
	};

	struct MaterialBlock
	{
		static const unsigned int BindingPoint = 3;
	};
}
