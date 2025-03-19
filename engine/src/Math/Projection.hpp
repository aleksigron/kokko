#pragma once

#include "Math/Mat4x4.hpp"

namespace kokko
{

enum class ProjectionType
{
	Perspective,
	Orthographic
};

struct ProjectionParameters
{
	// Type of projection to use
	ProjectionType projection = ProjectionType::Perspective;

	// The ratio of the viewport's width to its height (x / y)
	float aspect = 1.0f;

	// Perspective: vertical field of view in radians
	float perspectiveFieldOfView = 1.0f;
	float perspectiveNear = 0.1f;
	float perspectiveFar = 100.0f;

	// Orthographic: height of the frustum
	float orthographicHeight = 2.0f;
	float orthographicNear = 0.0f;
	float orthographicFar = 1.0f;

	Mat4x4f GetProjectionMatrix(bool reverseDepth) const
	{
		Mat4x4f result;

		if (projection == ProjectionType::Perspective)
		{
			float tanHalfFovy = std::tan(perspectiveFieldOfView * 0.5f);

			if (reverseDepth)
			{
				result[0] = 1.0f / (aspect * tanHalfFovy);
				result[5] = 1.0f / tanHalfFovy;
				result[10] = -perspectiveFar / (perspectiveNear - perspectiveFar) - 1.0f;
				result[11] = -1.0f;
				result[14] = -(perspectiveNear * perspectiveFar) / (perspectiveNear - perspectiveFar);
				result[15] = 0.0f;
			}
			else
			{
				result[0] = 1.0f / (aspect * tanHalfFovy);
				result[5] = 1.0f / tanHalfFovy;
				result[10] = -(perspectiveFar + perspectiveNear) / (perspectiveFar - perspectiveNear);
				result[11] = -1.0f;
				result[14] = (-2.0f * perspectiveFar * perspectiveNear) / (perspectiveFar - perspectiveNear);
				result[15] = 0.0f;
			}
		}
		else if (projection == ProjectionType::Orthographic)
		{
			float w = 2.0f / (orthographicHeight * aspect);
			float h = 2.0f / orthographicHeight;

			if (reverseDepth)
			{
				result[0] = w;
				result[5] = h;
				result[10] = -1.0f / (orthographicNear - orthographicFar);
				result[14] = -orthographicFar / (orthographicNear - orthographicFar);
			}
			else
			{
				result[0] = w;
				result[5] = h;
				result[10] = -1.0f / (orthographicFar - orthographicNear);
				result[14] = -orthographicNear / (orthographicFar - orthographicNear);
			}
		}

		return result;
	}

	void SetPerspective(float fovVerticalRadians)
	{
		projection = ProjectionType::Perspective;
		perspectiveFieldOfView = fovVerticalRadians;
	}

	void SetOrthographic(float height)
	{
		projection = ProjectionType::Orthographic;
		orthographicHeight = height;
	}

	void SetAspectRatio(float width, float height) { aspect = width / height; }
	void SetAspectRatio(int width, int height) { aspect = width / static_cast<float>(height); }
};

} // namespace kokko
