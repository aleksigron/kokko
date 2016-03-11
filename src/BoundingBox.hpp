#pragma once

#include "Vec3.hpp"
#include "Mat4x4.hpp"

#include <cmath>

struct BoundingBox
{
	Vec3f center;
	Vec3f extents;

	BoundingBox Transform(const Mat4x4f& m) const
	{
		Mat4x4f absm;

		for (int i = 0; i < 16; ++i)
			absm[i] = std::abs(m[i]);

		BoundingBox result;

		result.center = (Vec4f(this->center, 1.0f) * m).xyz();
		result.extents = (Vec4f(this->extents, 0.0f) * absm).xyz();

		return result;
	}
};
