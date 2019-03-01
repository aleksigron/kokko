#include "ViewFrustum.hpp"

#include <cmath>

#include "Vec3.hpp"
#include "Vec4.hpp"
#include "Mat4x4.hpp"
#include "BoundingBox.hpp"

#include "Camera.hpp"

void ViewFrustum::UpdateFrustum(const Camera& camera, const Mat4x4f& cameraTransform)
{
	const Vec3f position = (cameraTransform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
	const Vec3f forward = (cameraTransform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
	const Vec3f right = (cameraTransform * Vec4f(1.0f, 0.0f, 0.0f, 0.0f)).xyz();
	const Vec3f up = (cameraTransform * Vec4f(0.0f, 1.0f, 0.0f, 0.0f)).xyz();

	const float near = camera.nearClipDistance;
	const float far = camera.farClipDistance;

	const float halfFovTan = std::tan(camera.perspectiveFieldOfView * 0.5f);

	const float halfNearHeight = halfFovTan * near;
	const float halfNearWidth = halfNearHeight * camera.aspectRatio;

	const float halfFarHeight = halfFovTan * far;
	const float halfFarWidth = halfFarHeight * camera.aspectRatio;

	const Vec3f farForward = forward * far;
	const Vec3f halfFarRight = right * halfFarWidth;
	const Vec3f halfFarUp = up * halfFarHeight;

	const Vec3f dirTopLeft = farForward - halfFarRight + halfFarUp;
	const Vec3f dirBottomRight = farForward + halfFarRight - halfFarUp;

	const Vec3f nearCenter = position + forward * near;
	const Vec3f farCenter = position + farForward;

	// Order: near, left, top, right, bottom, far

	planes[0].SetPointAndNormal(nearCenter, forward);
	planes[1].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, up).GetNormalized());
	planes[2].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, right).GetNormalized());
	planes[3].SetPointAndNormal(position, Vec3f::Cross(up, dirBottomRight).GetNormalized());
	planes[4].SetPointAndNormal(position, Vec3f::Cross(right, dirBottomRight).GetNormalized());
	planes[5].SetPointAndNormal(farCenter, -forward);

	const Vec3f halfNearRight = right * halfNearWidth;
	const Vec3f halfNearUp = up * halfNearHeight;

	points[0] = nearCenter + halfNearUp - halfNearRight; // Near top left
	points[1] = nearCenter + halfNearUp + halfNearRight; // Near top right
	points[2] = nearCenter - halfNearUp - halfNearRight; // Near bottom left
	points[3] = nearCenter - halfNearUp + halfNearRight; // Near bottom right
	points[4] = farCenter + halfFarUp - halfFarRight; // Far top left
	points[5] = farCenter + halfFarUp + halfFarRight; // Far top right
	points[6] = farCenter - halfFarUp - halfFarRight; // Far bottom left
	points[7] = farCenter - halfFarUp + halfFarRight; // Far bottom right
}
