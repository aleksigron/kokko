#pragma once

#include "Math/Vec3.hpp"
#include "Math/Mat4x4.hpp"

struct BoundingBox
{
	Vec3f center;
	Vec3f extents;

	BoundingBox Transform(const Mat4x4f& m) const;
	void UpdateToContain(unsigned int count, const Vec3f* points);
};
