#pragma once

#include "Math/Mat4x4.hpp"
#include "Math/Rectangle.hpp"
#include "Math/Vec3.hpp"
#include "Math/Frustum.hpp"

struct RenderViewport
{
	Vec3f position;
	Vec3f forward;

	float farMinusNear;
	float minusNear;
	float objectMinScreenSizePx;

	Mat4x4fBijection view;
	Mat4x4f projection;
	Mat4x4f viewProjection;

	Rectanglei viewportRectangle;

	FrustumPlanes frustum;

	unsigned int uniformBlockObject;
};
