#pragma once

#include <cstdint>

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"

namespace UniformBuffer
{
	class LightingBlock
	{
	public:
		static const std::size_t MaxLightCount = 8;
		static const std::size_t MaxCascadeCount = 4;
		static const std::size_t BufferSize = 944;

	private:
		unsigned char data[BufferSize];

		void SetInt(std::size_t offset, int value)
		{
			*reinterpret_cast<int*>(data + offset) = value;
		}

		void SetFloat(std::size_t offset, float value)
		{
			*reinterpret_cast<float*>(data + offset) = value;
		}

		void SetFloatArray(std::size_t offset, std::size_t count, const float* values)
		{
			for (size_t i = 0; i < MaxLightCount; ++i)
				*reinterpret_cast<float*>(data + offset + i * 16) = values[i];
		}

		void SetVec2f(std::size_t offset, const Vec2f& value)
		{
			*reinterpret_cast<float*>(data + offset + 0) = value.x;
			*reinterpret_cast<float*>(data + offset + 4) = value.y;
		}

		void SetVec3f(std::size_t offset, const Vec3f& value)
		{
			*reinterpret_cast<float*>(data + offset + 0) = value.x;
			*reinterpret_cast<float*>(data + offset + 4) = value.y;
			*reinterpret_cast<float*>(data + offset + 8) = value.z;
		}

		void SetVec3fArray(std::size_t offset, std::size_t count, const Vec3f* values)
		{
			for (size_t i = 0; i < count; ++i)
				for (size_t j = 0; j < 3; ++j)
					*reinterpret_cast<float*>(data + offset + i * 16 + j * 4) = values[i][j];
		}

		void SetMat4x4f(std::size_t offset, const Mat4x4f& value)
		{
			for (size_t i = 0; i < 16; ++i)
				*reinterpret_cast<float*>(data + offset + i * 4) = value[i];
		}

		void SetMat4x4fArray(std::size_t offset, std::size_t count, const Mat4x4f* values)
		{
			for (size_t i = 0; i < count; ++i)
				for (size_t j = 0; j < 16; ++j)
					*reinterpret_cast<float*>(data + offset + i * 64 + j * 4) = values[i][j];
		}

	public:
		unsigned char* GetData() { return data; }
		const unsigned char* GetData() const { return data; }

		void SetPointLightCount(int pointLightCount) { SetInt(0, pointLightCount); }
		void SetSpotLightCount(int spotLightCount) { SetInt(4, spotLightCount); }
		void SetShadowCascadeCount(int cascadeCount) { SetInt(8, cascadeCount); }
		void SetHalfNearPlane(const Vec2f& halfNearPlane) { SetVec2f(16, halfNearPlane); }
		void SetPerspectiveMatrix(const Mat4x4f& matrix) { SetMat4x4f(32, matrix); }

		void SetLightColor(std::size_t index, const Vec3f& color) { SetVec3f(96 + index * 16, color); }
		void SetLightColors(std::size_t count, const Vec3f* colors) { SetVec3fArray(96, count, colors); }

		void SetLightPosition(std::size_t index, const Vec3f& position) { SetVec3f(224 + index * 16, position); }
		void SetLightPositions(std::size_t count, const Vec3f* positions) { SetVec3fArray(224, count, positions); }

		void SetLightDirection(std::size_t index, const Vec3f& direction) { SetVec3f(352 + index * 16, direction); }
		void SetLightDirections(std::size_t count, const Vec3f* directions) { SetVec3fArray(352, count, directions); }

		void SetLightAngle(std::size_t index, float angle) { SetFloat(480 + index * 16, angle); }
		void SetLightAngles(std::size_t count, const float* angles) { SetFloatArray(480, count, angles); }

		void SetShadowMatrix(std::size_t index, const Mat4x4f& matrix) { SetMat4x4f(608 + index * 64, matrix); }
		void SetShadowMatrices(std::size_t count, const Mat4x4f* matrices) { SetMat4x4fArray(608, count, matrices); }

		void SetShadowSplit(std::size_t index, float split) { SetFloat(864 + index * 16, split); }
		void SetShadowSplits(std::size_t count, const float* splits) { SetFloatArray(864, count, splits); }
	};
}
