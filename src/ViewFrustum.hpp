#pragma once

#include "Plane.hpp"
#include "Vec3.hpp"
#include "Mat4x4.hpp"
#include "Projection.hpp"

struct FrustumPlanes
{
	// Order: near, left, top, right, bottom, far
	Plane planes[6];

	void Update(const ProjectionParameters& params, const Mat4x4f& transform)
	{
		const Vec3f position = (transform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
		const Vec3f forward = (transform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
		const Vec3f right = (transform * Vec4f(1.0f, 0.0f, 0.0f, 0.0f)).xyz();
		const Vec3f up = (transform * Vec4f(0.0f, 1.0f, 0.0f, 0.0f)).xyz();

		const Vec3f farForward = forward * params.far;

		const Vec3f nearCenter = position + forward * params.near;
		const Vec3f farCenter = position + farForward;

		if (params.projection == ProjectionType::Perspective)
		{
			const float halfFovTan = std::tan(params.height * 0.5f);
			const float halfFarHeight = halfFovTan * params.far;
			const float halfFarWidth = halfFarHeight * params.aspect;

			const Vec3f halfFarRight = right * halfFarWidth;
			const Vec3f halfFarUp = up * halfFarHeight;

			const Vec3f dirTopLeft = farForward - halfFarRight + halfFarUp;
			const Vec3f dirBottomRight = farForward + halfFarRight - halfFarUp;

			planes[0].SetPointAndNormal(nearCenter, forward);
			planes[1].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, up).GetNormalized());
			planes[2].SetPointAndNormal(position, Vec3f::Cross(dirTopLeft, right).GetNormalized());
			planes[3].SetPointAndNormal(position, Vec3f::Cross(up, dirBottomRight).GetNormalized());
			planes[4].SetPointAndNormal(position, Vec3f::Cross(right, dirBottomRight).GetNormalized());
			planes[5].SetPointAndNormal(farCenter, -forward);
		}
		else if (params.projection == ProjectionType::Orthographic)
		{
			const float halfHeight = params.height * 0.5f;
			const float halfWidth = halfHeight * params.aspect;

			planes[0].SetPointAndNormal(nearCenter, forward);
			planes[1].SetPointAndNormal(position + right * -halfWidth, right);
			planes[2].SetPointAndNormal(position + up * halfHeight, -up);
			planes[3].SetPointAndNormal(position + right * halfWidth, -right);
			planes[4].SetPointAndNormal(position + up * -halfHeight, up);
			planes[5].SetPointAndNormal(farCenter, -forward);
		}
	}
};

struct FrustumPoints
{
	Vec3f points[8];

	void Update(const ProjectionParameters& params, const Mat4x4f& transform)
	{
		const Vec3f position = (transform * Vec4f(0.0f, 0.0f, 0.0f, 1.0f)).xyz();
		const Vec3f forward = (transform * Vec4f(0.0f, 0.0f, -1.0f, 0.0f)).xyz();
		const Vec3f right = (transform * Vec4f(1.0f, 0.0f, 0.0f, 0.0f)).xyz();
		const Vec3f up = (transform * Vec4f(0.0f, 1.0f, 0.0f, 0.0f)).xyz();

		float halfNearHeight;
		float halfFarHeight;

		if (params.projection == ProjectionType::Perspective)
		{
			const float halfFovTan = std::tan(params.height * 0.5f);

			halfNearHeight = halfFovTan * params.near;
			halfFarHeight = halfFovTan * params.far;
		}
		else // if (params.projection == ProjectionType::Orthographic)
		{
			halfNearHeight = params.height;
			halfFarHeight = params.height;
		}

		const float halfNearWidth = halfNearHeight * params.aspect;
		const float halfFarWidth = halfFarHeight * params.aspect;

		const Vec3f farForward = forward * params.far;
		const Vec3f halfFarRight = right * halfFarWidth;
		const Vec3f halfFarUp = up * halfFarHeight;

		const Vec3f nearCenter = position + forward * params.near;
		const Vec3f farCenter = position + farForward;

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
};
