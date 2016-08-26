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

	const float halfFarHeight = std::tan(camera.perspectiveFieldOfView * 0.5f) * far;
	const float halfFarWidth = halfFarHeight * camera.aspectRatio;

	const Vec3f farForward = forward * far;
	const Vec3f farRight = right * halfFarWidth;
	const Vec3f farUp = up * halfFarHeight;

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
