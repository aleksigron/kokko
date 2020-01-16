#pragma once

#include "Math/Vec3.hpp"

struct BoundingBox;
struct FrustumPlanes;
struct BitPack;

namespace Intersect
{
	void FrustumAABB(
		const FrustumPlanes& frustum,
		unsigned int count,
		const BoundingBox* bounds,
		BitPack* intersectedOut);

	void FrustumSphere(
		const FrustumPlanes& frustum,
		unsigned int count,
		const Vec3f* positions,
		const float* radii,
		BitPack* intersectedOut);
}
