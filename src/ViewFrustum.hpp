#pragma once

#include "Plane.hpp"
#include "Vec3.hpp"

class Camera;
struct Mat4x4f;
struct BoundingBox;

struct ViewFrustum
{
	Plane planes[6];
	Vec3f points[8];

	void UpdateFrustum(const Camera& camera, const Mat4x4f& cameraTransform);
};
