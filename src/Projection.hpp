#pragma once

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

	Mat4x4f GetProjectionMatrix() const
	{
		Mat4x4f result;

		const float farMinusNear = far - near;
		const float farPlusNear = far + near;

		if (projection == ProjectionType::Perspective)
		{
			const float tanHalfFovy = std::tan(height * 0.5f);

			result[0] = 1.0f / (aspect * tanHalfFovy);
			result[5] = 1.0f / tanHalfFovy;
			result[10] = -farPlusNear / farMinusNear;
			result[11] = - 1.0f;
			result[14] = - (2.0f * far * near) / farMinusNear;
			result[15] = 0.0f;
		}
		else if (projection == ProjectionType::Orthographic)
		{
			const float halfHeight = height * 0.5f;
			const float halfWidth = halfHeight * aspect;

			result[0] = 1.0f / halfWidth;
			result[5] = 1.0f / halfHeight;
			result[10] = -2.0f / farMinusNear;
			result[14] = -farPlusNear / farMinusNear;
		}

		return result;
	}

	void SetAspectRatio(float width, float height) { aspect = width / height; }
};
