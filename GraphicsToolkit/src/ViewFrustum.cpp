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
	const Vec3f right = camera.transform.Right();
	const Vec3f up = camera.transform.Up();

	const float tanHalfFov = tanf(camera.perspectiveFieldOfView * 0.5f);

	const float halfNearHeight = tanHalfFov * camera.nearClipDistance;
	const float halfNearWidth = halfNearHeight * camera.aspectRatio;

	const Vec3f nearCenter = position + forward * camera.nearClipDistance;
	const Vec3f nearRight = right * halfNearWidth;
	const Vec3f nearUp = up * halfNearHeight;

	const Vec3f dirTopLeft = (nearCenter - nearRight + nearUp - position).Normalized();
	const Vec3f dirBottomRight = (nearCenter + nearRight - nearUp - position).Normalized();

	// Order: near, left, top, right, bottom, far

	planes[0].SetPointAndNormal(nearCenter, forward);
	planes[1].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, up));
	planes[2].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, right));
	planes[3].SetPointAndNormal(position, Vec3f::Cross(up, dirBottomRight));
	planes[4].SetPointAndNormal(position, Vec3f::Cross(right, dirBottomRight));
	planes[5].SetPointAndNormal(position + forward * camera.farClipDistance, -forward);
}

void ViewFrustum::CullAABB(unsigned int boxCount, const BoundingBox* boxes, unsigned char* state) const
{
	const Plane* pl = this->planes;
	Vec3f absNormal[6];

	for (int i = 0; i < 6; ++i)
	{
		absNormal[i].x = fabsf(pl[i].normal.x);
		absNormal[i].y = fabsf(pl[i].normal.y);
		absNormal[i].z = fabsf(pl[i].normal.z);
	}

	for (unsigned int boxIndex = 0; boxIndex < boxCount; ++boxIndex)
	{
		const BoundingBox* const box = boxes + boxIndex;

		// Inside: 2, intersect: 1, outside: 0
		unsigned char result = 2;

		for (unsigned int planeIndex = 0; planeIndex < 6; ++planeIndex)
		{
			const float planeDistNeg = -planes[planeIndex].distance;
			const float d = Vec3f::Dot(box->center, planes[planeIndex].normal);
			const float r = Vec3f::Dot(box->extents, absNormal[planeIndex]);

			if (d + r < planeDistNeg)
			{
				result = 0;
				break;
			}
			else if (d - r < planeDistNeg)
				result = 1;
		}

		state[boxIndex] = result;
	}
}
