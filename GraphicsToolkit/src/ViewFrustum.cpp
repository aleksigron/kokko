#include "ViewFrustum.h"

#include <cmath>

#include "Vec3.h"
#include "Camera.h"
#include "Plane.h"
#include "BoundingBox.h"

void ViewFrustum::UpdateFrustum(const Camera& camera)
{
	const Vec3f position = camera.transform.position;
	const Vec3f forward = camera.transform.Forward();

	const float near = camera.nearClipDistance;
	const float far = camera.farClipDistance;

	const float halfFarHeight = std::tan(camera.perspectiveFieldOfView * 0.5f) * far;
	const float halfFarWidth = halfFarHeight * camera.aspectRatio;

	const Vec3f farForward = forward * far;
	const Vec3f farRight = camera.transform.Right() * halfFarWidth;
	const Vec3f farUp = camera.transform.Up() * halfFarHeight;

	const Vec3f dirTopLeft = farForward - farRight + farUp;
	const Vec3f dirBottomRight = farForward + farRight - farUp;

	// Order: near, left, top, right, bottom, far

	planes[0].SetPointAndNormal(position + forward * near, forward);
	planes[1].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, farUp).GetNormalized());
	planes[2].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, farRight).GetNormalized());
	planes[3].SetPointAndNormal(position, Vec3f::Cross(farUp, dirBottomRight).GetNormalized());
	planes[4].SetPointAndNormal(position, Vec3f::Cross(farRight, dirBottomRight).GetNormalized());
	planes[5].SetPointAndNormal(position + farForward, -forward);
}

void ViewFrustum::CullAABB(unsigned int count, const BoundingBox* boxes, unsigned char* state) const
{
	const Plane* frustumPlanes = this->planes;
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
