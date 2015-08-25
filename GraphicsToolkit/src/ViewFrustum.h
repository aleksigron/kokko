#pragma once

#include "Plane.h"

struct Camera;
struct BoundingBox;

struct ViewFrustum
{
	Plane planes[6];

	void UpdateFrustum(const Camera& camera);
	void Cull(unsigned int boxCount, const BoundingBox* boxes, unsigned char* state) const;
};




