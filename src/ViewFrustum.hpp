#pragma once

#include "Plane.hpp"

struct Camera;
struct BoundingBox;

struct ViewFrustum
{
	Plane planes[6];

	void UpdateFrustum(const Camera& camera);
};
