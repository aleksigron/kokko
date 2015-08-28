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
}
