#include "FrustumCulling.hpp"

#include <cmath>

#include "Vec3.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"
#include "CullState.hpp"

void FrustumCulling::CullAABB(const ViewFrustum* frustum,
							  unsigned int count,
							  const BoundingBox* bounds,
							  CullStatePacked16* state)
{
	const Plane* planes = frustum->planes;
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
		CullState result = CullState::Inside;

		// For each plane in view frustum
		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const float planeDistNeg = -planes[planeIdx].distance;
			const float d = Vec3f::Dot(bounds[boxIdx].center, planes[planeIdx].normal);
			const float r = Vec3f::Dot(bounds[boxIdx].extents, planeNormalAbs[planeIdx]);

			if (d + r < planeDistNeg)
			{
				result = CullState::Outside;
				break;
			}
			else if (d - r < planeDistNeg)
				result = CullState::Intersect;
		}

		CullStatePacked16::Set(state, boxIdx, result);
	}
}
