#include "CascadedShadowMap.hpp"

#include <cmath>
#include <algorithm>

#include "Math.hpp"
#include "ViewFrustum.hpp"
#include "BoundingBox.hpp"

namespace CascadedShadowMap
{
int GetShadowCascadeResolution()
{
	return 1024;
}

void CalculateCascadeFrusta(
	const Vec3f& lightDirection,
	const Mat4x4f& cameraTransform,
	const ProjectionParameters& projection,
	Mat4x4f* transformsOut,
	ProjectionParameters* projectionsOut)
{
	// Calculate split depths

	float splits[CascadeCount];
	CalculateSplitDepths(projection, splits);

	// Calculate frusta for camera perspective

	ProjectionParameters cascadeCameraFrusta[CascadeCount];
	FrustumPoints frustumPointsWs[CascadeCount];

	for (unsigned int i = 0; i < CascadeCount; ++i)
	{
		cascadeCameraFrusta[i].projection = projection.projection;
		cascadeCameraFrusta[i].aspect = projection.aspect;
		cascadeCameraFrusta[i].height = projection.height;
		cascadeCameraFrusta[i].near = i > 0 ? splits[i - 1] : projection.near;
		cascadeCameraFrusta[i].far = splits[i];
		frustumPointsWs[i].Update(cascadeCameraFrusta[i], cameraTransform);
	}

	// Calculate frusta for light perspective

	Vec3f upDir(0.0f, 1.0f, 0.0f);

	if (std::fabsf(Vec3f::Dot(lightDirection, upDir)) > 0.99f)
		upDir = Vec3f(0.0f, 0.0f, 1.0f);

	Vec3f axisZ = -lightDirection;

	Vec3f axisX = Vec3f::Cross(axisZ, upDir);
	axisX.Normalize();

	Vec3f axisY = Vec3f::Cross(axisX, axisZ);

	Mat3x3f orientation = Mat3x3f::FromAxes(axisX, axisY, axisZ);
	Mat4x4f rotatedView = Mat4x4f(orientation).GetInverse();

	for (unsigned int cascIdx = 0; cascIdx < CascadeCount; ++cascIdx)
	{
		Vec3f frustumCenter;
		for (unsigned int i = 0; i < 8; ++i)
			frustumCenter += frustumPointsWs[cascIdx].points[i];
		
		frustumCenter = frustumCenter * 0.125f;

		float radius = 0.0f;
		for (unsigned int i = 0; i < 8; ++i)
		{
			float distance = (frustumPointsWs[cascIdx].points[i] - frustumCenter).Magnitude();
			radius = std::max(radius, distance);
		}

		Vec3f at = frustumCenter - lightDirection * radius;
		transformsOut[cascIdx] = Mat4x4f::LookAt(at, frustumCenter, Vec3f(0.0f, 1.0f, 0.0f));
		
		ProjectionParameters& cascadeProjection = projectionsOut[cascIdx];
		cascadeProjection.projection = ProjectionType::Orthographic;
		cascadeProjection.aspect = 1.0f;
		cascadeProjection.height = radius;
		cascadeProjection.near = 0.0f;
		cascadeProjection.far = radius * 2.0f;
	}
}

void CalculateSplitDepths(const ProjectionParameters& projection, float* depthsOut)
{
	CalculateSplitDepths(projection.near, projection.far, depthsOut);
}

void CalculateSplitDepths(float near, float far, float* depthsOut)
{
	const float maxShadowDistance = 100.0f;
	const float shadowSplitLogFactor = 0.9f;

	far = std::min(maxShadowDistance, far);

	float i_f = 1.0f;
	float fCascadeCount = static_cast<float>(CascadeCount);

	for (unsigned int i = 0; i < CascadeCount - 1; i++, i_f += 1.0f)
	{
		depthsOut[i] = Math::Lerp(
			near + (i_f / fCascadeCount) * (far - near),
			near * std::powf(far / near, i_f / fCascadeCount),
			shadowSplitLogFactor);
	}
	depthsOut[CascadeCount - 1] = far;
}

} // namespace CascadedShadowMap 
