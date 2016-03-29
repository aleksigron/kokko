#include "ViewFrustum.hpp"

#include <cmath>

#include "Vec3.hpp"
#include "Camera.hpp"
#include "BoundingBox.hpp"

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
