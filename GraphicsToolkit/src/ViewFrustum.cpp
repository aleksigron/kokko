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

void ViewFrustum::Cull(unsigned int boxCount, const BoundingBox* boxes, unsigned char* state) const
{
	Vec3f absNormal[6];

	for (int i = 0; i < 6; ++i)
		absNormal[i] = Vec3f::ComponentWiseAbs(planes[i].normal);

	for (unsigned int boxIndex = 0; boxIndex < boxCount; ++boxIndex)
	{
		const Vec3f& center = boxes[boxIndex].center;
		const Vec3f& extent = boxes[boxIndex].extents;

		// Inside: 2, intersect: 1, outside: 0
		unsigned char result = 2;

		for (unsigned int planeIndex = 0; planeIndex < 6; ++planeIndex)
		{
			const Plane& pl = planes[planeIndex];
			const Vec3f& absNor = absNormal[planeIndex];

			const float d = center.x * pl.normal.x + center.y * pl.normal.y + center.z * pl.normal.z;
			const float r = extent.x * absNor.x + extent.y * absNor.y + extent.z * absNor.z;

			if (d + r < -pl.distance)
			{
				result = 0;
				break;
			}
			else if (d - r < -pl.distance)
				result = 1;
		}

		state[boxIndex] = result;
	}
}
