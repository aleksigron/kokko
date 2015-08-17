#pragma once

#include "Transform.h"
#include "Mat4x4.h"
#include "Vec2.h"
#include "Math.h"

struct Camera
{
	enum class Projection
	{
		Perspective,
		Orthographic
	};

	Transform transform;

	// The camera's vertical field of view in radians
	float perspectiveFieldOfView = Mathf::DegreesToRadians(45.0f);

	// The height of the camera's orthogonal viewport
	float orthogonalHeight = 1.0f;

	// The ratio of the viewport's width to its height (x / y)
	float aspectRatio = 1.0f;

	float nearClipDistance = 1.0f;
	float farClipDistance = 100.0f;
	
	Projection projectionType = Projection::Perspective;

	inline Mat4x4f GetProjectionMatrix()
	{
		if (projectionType == Projection::Perspective)
			return Matrix::Perspective(perspectiveFieldOfView, aspectRatio,
											 nearClipDistance, farClipDistance);

		else if (projectionType == Projection::Orthographic)
			return Matrix::Orthographic(orthogonalHeight / aspectRatio * 0.5f,
										orthogonalHeight * 0.5f,
										nearClipDistance, farClipDistance);

		return Mat4x4f();
	}

	inline Mat4x4f GetViewProjectionMatrix()
	{
		return this->GetProjectionMatrix() * this->GetViewMatrix();
	}

	inline Mat4x4f GetViewMatrix()
	{
		return Matrix::Transpose(transform.rotation) * Matrix::Translate(-(transform.position));
	}

	inline void SetAspectRatio(int width, int height)
	{
		aspectRatio = width / static_cast<float>(height);
	}
};