#include "Intersect3D.hpp"

#include <cmath>

#include "Core/BitPack.hpp"

#include "Math/BoundingBox.hpp"
#include "Math/Frustum.hpp"
#include "Math/Mat4x4.hpp"

#include "Engine/Engine.hpp"
#include "Debug/Debug.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"
#include "Core/String.hpp"

void Intersect::FrustumAABB(
	const FrustumPlanes& frustum,
	unsigned int count,
	const BoundingBox* bounds,
	BitPack* intersectedOut)
{
	const Plane* planes = frustum.planes;
	Vec3f planeNormalAbs[6];

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(planes[i].normal.x);
		planeNormalAbs[i].y = std::abs(planes[i].normal.y);
		planeNormalAbs[i].z = std::abs(planes[i].normal.z);
	}

	// For each axis aligned bounding box
	for (unsigned int boxIdx = 0; boxIdx < count; ++boxIdx)
	{
		bool inside = true;

		// For each plane in view frustum
		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const float d = Vec3f::Dot(bounds[boxIdx].center, planes[planeIdx].normal);
			const float r = Vec3f::Dot(bounds[boxIdx].extents, planeNormalAbs[planeIdx]);

			if (d + r < -planes[planeIdx].distance)
			{
				inside = false;
				break;
			}
		}

		BitPack::Set(intersectedOut, boxIdx, inside);
	}
}

void Intersect::FrustumAABBMinSize(
	const FrustumPlanes& frustum,
	const Mat4x4f& viewProjection,
	float minimumSize,
	unsigned int count,
	const BoundingBox* bounds,
	BitPack* intersectedOut)
{
	Debug* debug = Engine::GetInstance()->GetDebug();
	DebugTextRenderer* tr = debug->GetTextRenderer();
	DebugVectorRenderer* vr = debug->GetVectorRenderer();
	char strBuffer[128];

	const Plane* planes = frustum.planes;
	Vec3f planeNormalAbs[6];
	Vec3f boxCornerMultipliers[8];
	Vec3f half3(0.5f, 0.5f, 0.0f);

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(planes[i].normal.x);
		planeNormalAbs[i].y = std::abs(planes[i].normal.y);
		planeNormalAbs[i].z = std::abs(planes[i].normal.z);
	}

	for (int i = 0; i < 8; ++i)
	{
		boxCornerMultipliers[i].x = ((i % 2) * 2 - 1);
		boxCornerMultipliers[i].y = (((i / 2) % 2) * 2 - 1);
		boxCornerMultipliers[i].z = (((i / 4) % 2) * 2 - 1);
	}

	// For each axis aligned bounding box
	for (unsigned int boxIdx = 0; boxIdx < count; ++boxIdx)
	{
		bool visible = true;

		Vec3f center = bounds[boxIdx].center;
		Vec3f extents = bounds[boxIdx].extents;

		// For each plane in view frustum
		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const float d = Vec3f::Dot(center, planes[planeIdx].normal);
			const float r = Vec3f::Dot(extents, planeNormalAbs[planeIdx]);

			if (d + r < -planes[planeIdx].distance)
			{
				visible = false;
				break;
			}
		}

		if (visible == true)
		{
			Vec2f min(1e9f, 1e9f);
			Vec2f max(-1e9f, -1e9f);

			for (unsigned cornerIdx = 0; cornerIdx < 8; ++cornerIdx)
			{
				Vec3f corner = Vec3f::Hadamard(extents, boxCornerMultipliers[cornerIdx]);
				
				Vec4f proj = viewProjection * Vec4f(center + corner, 1.0f);
				Vec3f scr = proj.xyz() * (1.0f / proj.w) * 0.5f + half3;

				min.x = std::min(scr.x, min.x);
				min.y = std::min(scr.y, min.y);
				max.x = std::max(scr.x, max.x);
				max.y = std::max(scr.y, max.y);
			}

			float width = max.x - min.x;
			float height = max.y - min.y;

			if (width * height < minimumSize)
			{
				visible = false;
			}
		}

		BitPack::Set(intersectedOut, boxIdx, visible);
	}
}

void Intersect::FrustumSphere(
	const FrustumPlanes& frustum,
	unsigned int count,
	const Vec3f* positions,
	const float* radii,
	BitPack* intersectedOut)
{
	const Plane* planes = frustum.planes;
	Vec3f planeNormalAbs[6];

	for (int i = 0; i < 6; ++i)
	{
		planeNormalAbs[i].x = std::abs(planes[i].normal.x);
		planeNormalAbs[i].y = std::abs(planes[i].normal.y);
		planeNormalAbs[i].z = std::abs(planes[i].normal.z);
	}

	for (unsigned int sphereIdx = 0; sphereIdx < count; ++sphereIdx)
	{
		bool inside = true;

		for (unsigned int planeIdx = 0; planeIdx < 6; ++planeIdx)
		{
			const Vec3f& normal = frustum.planes[planeIdx].normal;
			float dist = frustum.planes[planeIdx].distance;
			float side = Vec3f::Dot(positions[sphereIdx], normal) + dist;

			if (side < -radii[sphereIdx])
				inside = false;
		}

		BitPack::Set(intersectedOut, sphereIdx, inside);
	}
}
