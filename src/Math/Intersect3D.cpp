#include "Intersect3D.hpp"

#include <cmath>

#include "Math/Frustum.hpp"
#include "Math/BoundingBox.hpp"
#include "Core/BitPack.hpp"

void Intersect::FrustumAABB(
	const FrustumPlanes& frustum,
	unsigned int count,
	const BoundingBox* bounds,
	BitPack* intersectedOut)
{
	const Plane* planes = frustum.planes;
	Vec3f planeNormalAbs[6];

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(planes[i].normal.x);
		planeNormalAbs[i].y = std::abs(planes[i].normal.y);
		planeNormalAbs[i].z = std::abs(planes[i].normal.z);
	}

	// For each axis aligned bounding box
	for (unsigned int boxIdx = 0; boxIdx < count; ++boxIdx)
	{
		bool inside = true;

		// For each plane in view frustum
		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const float d = Vec3f::Dot(bounds[boxIdx].center, planes[planeIdx].normal);
			const float r = Vec3f::Dot(bounds[boxIdx].extents, planeNormalAbs[planeIdx]);

			if (d + r < -planes[planeIdx].distance)
			{
				inside = false;
				break;
			}
		}

		BitPack::Set(intersectedOut, boxIdx, inside);
	}
}

void Intersect::FrustumSphere(
	const FrustumPlanes& frustum,
	unsigned int count,
	const Vec3f* positions,
	const float* radii,
	BitPack* intersectedOut)
{
	const Plane* planes = frustum.planes;
	Vec3f planeNormalAbs[6];

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(planes[i].normal.x);
		planeNormalAbs[i].y = std::abs(planes[i].normal.y);
		planeNormalAbs[i].z = std::abs(planes[i].normal.z);
	}

	for (unsigned int sphereIdx = 0; sphereIdx < count; ++sphereIdx)
	{
		bool inside = true;

		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const Vec3f& normal = frustum.planes[planeIdx].normal;
			float dist = frustum.planes[planeIdx].distance;
			float side = Vec3f::Dot(positions[sphereIdx], normal) + dist;

			if (side < -radii[sphereIdx])
				inside = false;
		}

		BitPack::Set(intersectedOut, sphereIdx, inside);
	}
}
