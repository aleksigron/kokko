#pragma once

#include "Math/Vec3.hpp"

struct BoundingBox;
struct Mat4x4f;
struct FrustumPlanes;
struct BitPack;

namespace Intersect
{
	/*
	* Calculate visibility for a single bounding box
	*/
	bool FrustumAabb(
	const FrustumPlanes& frustum,
	const BoundingBox& bounds);

	/*
	* Calculate visibility for bounding boxes
	*/
	void FrustumAabbN(
		const FrustumPlanes& frustum,
		unsigned int count,
		const BoundingBox* bounds,
		BitPack* intersectedOut);

	/*
	* Calculate visibility for bounding boxes with a minimum size
	*/
	void FrustumAABBMinSize(
		const FrustumPlanes& frustum,
		const Mat4x4f& viewProjection,
		float minimumSize,
		unsigned int count,
		const BoundingBox* bounds,
		BitPack* intersectedOut);

	/*
	* Calculate visibility for spheres
	*/
	void FrustumSphere(
		const FrustumPlanes& frustum,
		unsigned int count,
		const Vec3f* positions,
		const float* radii,
		BitPack* intersectedOut);
}
