#pragma once

#include <cmath>

#include "Mat4x4.h"
#include "Vec3.h"

namespace Matrix
{
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

	inline Mat4x4f Transpose(const Mat4x4f& m)
	{
		Mat4x4f result(Mat4x4f::uninit);
		
		for (std::size_t i = 0; i < 16; ++i)
			result[i] = m[(i % 4) * 4 + i / 4];
		
		return result;
	}
}
