#pragma once

#include "Plane.hpp"

class Camera;
struct Mat4x4f;
struct BoundingBox;

struct ViewFrustum
{
	Plane planes[6];

	void UpdateFrustum(const Camera& camera, const Mat4x4f& cameraTransform);
};
