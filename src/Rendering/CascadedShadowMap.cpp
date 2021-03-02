#include "Rendering/CascadedShadowMap.hpp"

#include <cmath>
#include <algorithm>

#include "Math/Vec2.hpp"
#include "Math/Math.hpp"
#include "Math/Frustum.hpp"
#include "Math/BoundingBox.hpp"

namespace CascadedShadowMap
{
int GetShadowCascadeResolution()
{
	return 1024;
}

unsigned int GetCascadeCount()
{
	return 4;
}

void CalculateCascadeFrusta(
	const Vec3f& lightDirection,
	const Mat4x4f& cameraTransform,
	const ProjectionParameters& projection,
	Mat4x4f* transformsOut,
	ProjectionParameters* projectionsOut)
{
	float splits[MaxCascadeCount];
	CalculateSplitDepths(projection, splits);

	Vec3f up(0.0f, 1.0f, 0.0f);

	if (std::abs(Vec3f::Dot(lightDirection, up)) > 0.99f)
		up = Vec3f(0.0f, 0.0f, -1.0f);

	Mat4x4f placeholderLightTransform = Mat4x4f::LookAt(Vec3f(0.0f, 0.0f, 0.0f), lightDirection, up);
	Vec3f lightDirX = (placeholderLightTransform * Vec4f(1.0f, 0.0f, 0.0f, 0.0f)).xyz();
	Vec3f lightDirY = (placeholderLightTransform * Vec4f(0.0f, 1.0f, 0.0f, 0.0f)).xyz();

	Vec2f halfFovTan;
	halfFovTan.y = std::tan(projection.height * 0.5f);
	halfFovTan.x = halfFovTan.y * projection.aspect;
	float crossHalfFovTan = halfFovTan.Magnitude();

	for (unsigned int cascIdx = 0, count = GetCascadeCount(); cascIdx < count; ++cascIdx)
	{
		// Calculate frusta for camera perspective
		ProjectionParameters cascadeCameraFrustum;
		cascadeCameraFrustum.projection = projection.projection;
		cascadeCameraFrustum.aspect = projection.aspect;
		cascadeCameraFrustum.height = projection.height;
		cascadeCameraFrustum.near = cascIdx > 0 ? splits[cascIdx - 1] : projection.near;
		cascadeCameraFrustum.far = splits[cascIdx];

		FrustumPoints fp;
		fp.Update(cascadeCameraFrustum, cameraTransform);

		// Calculate frusta for light perspective
		// Calculate enclosing sphere

		Vec3f farPlaneCenter = (cameraTransform * Vec4f(0.0f, 0.0f, -splits[cascIdx], 1.0f)).xyz();

		float resolution = static_cast<float>(GetShadowCascadeResolution());
		float roundingMarginFactor = resolution / (resolution - 1.0f);

		Vec3f sphereCenter;
		float radius;

		// Near plane points are closer than far plane points
		if ((fp.points[0] - farPlaneCenter).SqrMagnitude() < (fp.points[4] - farPlaneCenter).SqrMagnitude())
		{
			// Optimal sphere center is at far plane center
			sphereCenter = farPlaneCenter;
			radius = (fp.points[4] - farPlaneCenter).Magnitude() * roundingMarginFactor;
		}
		else // Near plane points are farther than far plane points
		{
			// Need to calculate optimal position for sphere center
			// Consider as a 2D corner to corner cross-section of frustum

			// Average of near and far depths
			float cascadeHalfDepth = (cascadeCameraFrustum.far + cascadeCameraFrustum.near) * 0.5f;

			// Distance from center axis to frustum corner line at halfway depth
			float midHalfWidth = cascadeHalfDepth * crossHalfFovTan;

			// Sphere center offset from frustum center line halfway point
			float offsetFromFrustumCenter = midHalfWidth * crossHalfFovTan;

			// Sphere center depth from camera
			float sphereDepth = cascadeHalfDepth + offsetFromFrustumCenter;

			sphereCenter = (cameraTransform * Vec4f(0.0f, 0.0f, -sphereDepth, 1.0f)).xyz();
			radius = (sphereCenter - fp.points[0]).Magnitude() * roundingMarginFactor;
		}

		Vec3f from = sphereCenter - lightDirection * radius;

		Mat4x4f lightModelTransform = Mat4x4f::LookAt(from, sphereCenter, up);
		Mat4x4f lightViewTransform = lightModelTransform.GetInverse();

		float diameter = radius * 2.0f;

		const float frontShadowRenderingDistance = 500.0f;

		ProjectionParameters cascProj;
		cascProj.projection = ProjectionType::Orthographic;
		cascProj.aspect = 1.0f;
		cascProj.height = diameter;
		cascProj.near = -frontShadowRenderingDistance;
		cascProj.far = diameter;

		// Calculate rounding in shadow map space to remove edge shimmer

		Mat4x4f shadowMatrix = cascProj.GetProjectionMatrix() * lightViewTransform;
		Vec4f originShadowSpace = shadowMatrix * Vec4f(0.0f, 0.0f, 0.0f, 1.0f) * resolution * 0.5f;
		Vec4f roundedOrigin(std::round(originShadowSpace.x), std::round(originShadowSpace.y), 0.0f, 0.0f);
		Vec4f offsetShadowSpace = (roundedOrigin - originShadowSpace) * (2.0f / resolution);
		Vec3f offsetWs = (lightDirX * offsetShadowSpace.x + lightDirY * offsetShadowSpace.y) * radius;

		// Because we don't return projection as a matrix, we have to add the offset to the view transform

		transformsOut[cascIdx] = Mat4x4f::LookAt(from - offsetWs, sphereCenter - offsetWs, up);
		projectionsOut[cascIdx] = cascProj;
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
	unsigned int cascadeCountInt = GetCascadeCount();
	float cascadeCountFloat = static_cast<float>(cascadeCountInt);


	for (unsigned int i = 0; i < cascadeCountInt - 1; i++, i_f += 1.0f)
	{
		depthsOut[i] = Math::Lerp(
			near + (i_f / cascadeCountFloat) * (far - near),
			near * std::powf(far / near, i_f / cascadeCountFloat),
			shadowSplitLogFactor);
	}
	depthsOut[cascadeCountInt - 1] = far;
}

} // namespace CascadedShadowMap 
