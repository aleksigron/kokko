#pragma once

#include "Math/Mat4x4.hpp"

enum class ProjectionType
{
	Perspective,
	Orthographic
};

struct ProjectionParameters
{
	// Type of projection to use
	ProjectionType projection;

	// ProjectionType::Perspective: vertical field of view in radians
	// ProjectionType::Orthogonal: height of the frustum
	float height;

	// The ratio of the viewport's width to its height (x / y)
	float aspect;

	// The near plane distance
	float near;

	// The far plane distance
	float far;

	Mat4x4f GetProjectionMatrix(bool reverseDepth) const
	{
		Mat4x4f result;

		if (projection == ProjectionType::Perspective)
		{
			float tanHalfFovy = std::tan(height * 0.5f);

			if (reverseDepth)
			{
				result[0] = 1.0f / (aspect * tanHalfFovy);
				result[5] = 1.0f / tanHalfFovy;
				result[10] = -far / (near - far) - 1.0f;
				result[11] = -1.0f;
				result[14] = -(near * far) / (near - far);
				result[15] = 0.0f;
			}
			else
			{
				result[0] = 1.0f / (aspect * tanHalfFovy);
				result[5] = 1.0f / tanHalfFovy;
				result[10] = -(far + near) / (far - near);
				result[11] = -1.0f;
				result[14] = (-2.0f * far * near) / (far - near);
				result[15] = 0.0f;
			}
		}
		else if (projection == ProjectionType::Orthographic)
		{
			float w = 2.0f / (height * aspect);
			float h = 2.0f / height;

			if (reverseDepth)
			{
				result[0] = w;
				result[5] = h;
				result[10] = -1.0f / (near - far);
				result[14] = -far / (near - far);
			}
			else
			{
				result[0] = w;
				result[5] = h;
				result[10] = -1.0f / (far - near);
				result[14] = -near / (far - near);
			}
		}

		return result;
	}

	void SetPerspective(float fovVerticalRadians)
	{
		projection = ProjectionType::Perspective;
		height = fovVerticalRadians;
	}

	void SetOrthographic(float height)
	{
		projection = ProjectionType::Orthographic;
		height = height;
	}

	void SetAspectRatio(float width, float height) { aspect = width / height; }
};
