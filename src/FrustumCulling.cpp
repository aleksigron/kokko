#include "FrustumCulling.hpp"

#include <cmath>

#include "Vec3.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"

void FrustumCulling::CullAABB(const ViewFrustum* frustum,
							  unsigned int count,
							  const BoundingBox* boxes,
							  unsigned char* state)
{
	const Plane* frustumPlanes = frustum->planes;
	Vec3f planeNormalAbs[6];

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(frustumPlanes[i].normal.x);
		planeNormalAbs[i].y = std::abs(frustumPlanes[i].normal.y);
		planeNormalAbs[i].z = std::abs(frustumPlanes[i].normal.z);
	}

	// For each axis aligned bounding box
	for (unsigned int boxIdx = 0; boxIdx < count; ++boxIdx)
	{
		// Inside: 2, intersect: 1, outside: 0
		unsigned char result = 2;

		// For each plane in view frustum
		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const float planeDistNeg = -frustumPlanes[planeIdx].distance;
			const float d = Vec3f::Dot(boxes[boxIdx].center, frustumPlanes[planeIdx].normal);
			const float r = Vec3f::Dot(boxes[boxIdx].extents, planeNormalAbs[planeIdx]);

			if (d + r < planeDistNeg)
			{
				result = 0;
				break;
			}
			else if (d - r < planeDistNeg)
				result = 1;
		}

		state[boxIdx] = result;
	}
}
