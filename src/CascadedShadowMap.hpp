#pragma once

#include "Vec3.hpp"
#include "Mat4x4.hpp"
#include "Projection.hpp"

namespace CascadedShadowMap
{
	constexpr unsigned int MaxCascadeCount = 4;

	unsigned int GetCascadeCount();
	int GetShadowCascadeResolution();

	void CalculateCascadeFrusta(
		const Vec3f& lightDirection,
		const Mat4x4f& cameraTransform,
		const ProjectionParameters& projection,
		Mat4x4f* transformsOut,
		ProjectionParameters* projectionsOut);

	void CalculateSplitDepths(const ProjectionParameters& projection, float* depthsOut);
	void CalculateSplitDepths(float near, float far, float* depthsOut);
};
