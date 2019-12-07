#include "CascadedShadowMap.hpp"

#include <cmath>
#include <algorithm>

#include "Vec2.hpp"
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
	float splits[CascadeCount];
	CalculateSplitDepths(projection, splits);

	Vec2f halfFovTan;
	halfFovTan.y = std::tan(projection.height * 0.5f);
	halfFovTan.x = halfFovTan.y * projection.aspect;
	float crossHalfFovTan = halfFovTan.Magnitude();

	for (unsigned int cascIdx = 0; cascIdx < CascadeCount; ++cascIdx)
	{
		// Calculate frusta for camera perspective
		ProjectionParameters cascadeCameraFrustum;
		cascadeCameraFrustum.projection = projection.projection;
		cascadeCameraFrustum.aspect = projection.aspect;
		cascadeCameraFrustum.height = projection.height;
		cascadeCameraFrustum.near = cascIdx > 0 ? splits[cascIdx - 1] : projection.near;
		cascadeCameraFrustum.far = splits[cascIdx];

		// Average of near and far depths
		float cascadeHalfDepth = (cascadeCameraFrustum.far + cascadeCameraFrustum.near) * 0.5f;

		FrustumPoints fp;
		fp.Update(cascadeCameraFrustum, cameraTransform);

		// Calculate frusta for light perspective
		// Calculate enclosing sphere

		Vec3f farPlaneCenter = (cameraTransform * Vec4f(0.0f, 0.0f, -splits[cascIdx], 1.0f)).xyz();

		Vec3f sphereCenter;
		float radius;

		// Near plane points are closer than far plane points
		if ((fp.points[0] - farPlaneCenter).SqrMagnitude() < (fp.points[4] - farPlaneCenter).SqrMagnitude())
		{
			// Optimal sphere center is at far plane center
			sphereCenter = farPlaneCenter;
			radius = (fp.points[4] - farPlaneCenter).Magnitude();
		}
		else // Near plane points are farther than far plane points
		{
			// Need to calculate optimal position for sphere center
			// Consider as a 2D corner to corner cross-section of frustum

			// Distance from center axis to frustum corner line at halfway depth
			float midHalfWidth = cascadeHalfDepth * crossHalfFovTan;

			// Sphere center offset from frustum center line halfway point
			float offsetFromFrustumCenter = midHalfWidth * crossHalfFovTan;

			// Sphere center depth from camera
			float sphereDepth = cascadeHalfDepth + offsetFromFrustumCenter;

			sphereCenter = (cameraTransform * Vec4f(0.0f, 0.0f, -sphereDepth, 1.0f)).xyz();
			radius = (sphereCenter - fp.points[0]).Magnitude();
		}

		Vec3f from = sphereCenter - lightDirection * radius;
		transformsOut[cascIdx] = Mat4x4f::LookAt(from, sphereCenter, Vec3f(0.0f, 1.0f, 0.0f));

		float diameter = radius * 2.0f;
		
		ProjectionParameters& cascadeProjection = projectionsOut[cascIdx];
		cascadeProjection.projection = ProjectionType::Orthographic;
		cascadeProjection.aspect = 1.0f;
		cascadeProjection.height = diameter;
		cascadeProjection.near = 0.0f;
		cascadeProjection.far = diameter;
	}
}

void CalculateSplitDepths(const ProjectionParameters& projection, float* depthsOut)
{
	CalculateSplitDepths(projection.near, projection.far, depthsOut);
}

void CalculateSplitDepths(float near, float far, float* depthsOut)
{
	const float maxShadowDistance = 100.0f;
	const float shadowSplitLogFactor = 0.8f;

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
