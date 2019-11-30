#pragma once

#include "Vec3.hpp"
#include "Mat4x4.hpp"

#include <cmath>
#include <algorithm>
#include <limits>

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

		result.center = (m * Vec4f(this->center, 1.0f)).xyz();
		result.extents = (absm * Vec4f(this->extents, 0.0f)).xyz();

		return result;
	}

	void UpdateToContain(unsigned int count, const Vec3f* points)
	{
		constexpr float fmax = std::numeric_limits<float>::max();
		constexpr float fmin = std::numeric_limits<float>::min();

		Vec3f minimum(fmax, fmax, fmax);
		Vec3f maximum(fmin, fmin, fmin);

		for (unsigned int i = 0; i < count; ++i)
		{
			minimum.x = std::min(minimum.x, points[i].x);
			minimum.y = std::min(minimum.y, points[i].y);
			minimum.z = std::min(minimum.z, points[i].z);
			maximum.x = std::max(maximum.x, points[i].x);
			maximum.y = std::max(maximum.y, points[i].y);
			maximum.z = std::max(maximum.z, points[i].z);
		}

		Vec3f difference = maximum - minimum;

		extents = difference * 0.5f;
		center = minimum + extents;
	}
};
