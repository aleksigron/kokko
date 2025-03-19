#pragma once

#include "Math/Vec3.hpp"

namespace kokko
{

struct Plane
{
	Vec3f normal;
	float distance;

	inline void SetPointAndNormal(const Vec3f& p, const Vec3f& n)
	{
		this->normal = n;
		this->distance = -Vec3f::Dot(n, p);
	}
};

} // namespace kokko
