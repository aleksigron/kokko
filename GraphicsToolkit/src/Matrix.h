#pragma once

#include <cmath>

#include "Mat4x4.h"
#include "Vec3.h"

namespace Matrix
{
	inline Mat4x4f Perspective(float fovVertical, float aspectRatio, float near, float far)
	{
		float const tanHalfFovy = std::tan(fovVertical / 2.0f);
		
		Mat4x4f result;
		
		result[0] = 1.0f / (aspectRatio * tanHalfFovy);
		result[5] = 1.0f / (tanHalfFovy);
		result[10] = - (far + near) / (far - near);
		result[11] = - 1.0f;
		result[14] = - (2.0f * far * near) / (far - near);
		
		return result;
	}

	inline Mat4x4f Orthographic(float halfWidth, float halfHeight, float near, float far)
	{
		Mat4x4f result;
		
		result[0] = 1.0f / halfWidth;
		result[5] = 1.0f / halfHeight;
		result[10] = -2.0f / (far - near);
		result[14] = - (far + near) / (far - near);
		
		return result;
	}

	inline Mat4x4f Translate(const Vec3f& translation)
	{
		Mat4x4f result;
		
		result[12] = translation.x;
		result[13] = translation.y;
		result[14] = translation.z;
		
		return result;
	}

	inline Mat4x4f Scale(const Vec3f& scaling)
	{
		Mat4x4f result;
		
		result[0] = scaling.x;
		result[5] = scaling.y;
		result[10] = scaling.z;
		
		return result;
	}

	inline Mat3x3f Rotate3(Vec3f axis, float angle)
	{
		axis.Normalize();

		const float xx = axis.x * axis.x;
		const float yy = axis.y * axis.y;
		const float zz = axis.z * axis.z;

		const float xy = axis.x * axis.y;
		const float xz = axis.x * axis.z;
		const float yz = axis.y * axis.z;

		const float ca = std::cosf(angle);
		const float sa = std::sinf(angle);

		Mat3x3f result;

		result[0] = xx + (1.0f - xx) * ca;
		result[1] = xy * (1.0f - ca) + axis.z * sa;
		result[2] = xz * (1.0f - ca) - axis.y * sa;

		result[3] = xy * (1.0f - ca) - axis.z * sa;
		result[4] = yy + (1.0f - yy) * ca;
		result[5] = yz * (1.0f - ca) + axis.x * sa;

		result[6] = xz * (1.0f - ca) + axis.y * sa;
		result[7] = yz * (1.0f - ca) - axis.x * sa;
		result[8] = zz + (1.0f - zz) * ca;

		return result;
	}

	inline Mat4x4f Rotate(Vec3f axis, float angle)
	{
		axis.Normalize();
		
		const float xx = axis.x * axis.x;
		const float yy = axis.y * axis.y;
		const float zz = axis.z * axis.z;
		
		const float xy = axis.x * axis.y;
		const float xz = axis.x * axis.z;
		const float yz = axis.y * axis.z;
		
		const float ca = std::cosf(angle);
		const float sa = std::sinf(angle);
		
		Mat4x4f result;
		
		result[0] = xx + (1.0f - xx) * ca;
		result[1] = xy * (1.0f - ca) + axis.z * sa;
		result[2] = xz * (1.0f - ca) - axis.y * sa;
		
		result[4] = xy * (1.0f - ca) - axis.z * sa;
		result[5] = yy + (1.0f - yy) * ca;
		result[6] = yz * (1.0f - ca) + axis.x * sa;
		
		result[8] = xz * (1.0f - ca) + axis.y * sa;
		result[9] = yz * (1.0f - ca) - axis.x * sa;
		result[10] = zz + (1.0f - zz) * ca;
		
		return result;
	}

	inline Mat4x4f Transpose(const Mat4x4f& m)
	{
		Mat4x4f result(Mat4x4f::uninit);
		
		for (std::size_t i = 0; i < 16; ++i)
			result[i] = m[(i % 4) * 4 + i / 4];
		
		return result;
	}
}