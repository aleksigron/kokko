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

	float nearClipDistance = 0.1f;
	float farClipDistance = 100.0f;
	
	Projection projectionType = Projection::Perspective;

	Mat4x4f GetProjectionMatrix() const;

	inline Mat4x4f GetViewProjectionMatrix()
	{
		return this->GetProjectionMatrix() * this->GetViewMatrix();
	}

	inline Mat4x4f GetViewMatrix()
	{
		return Mat4x4f(transform.rotation.GetTransposed()) * Matrix::Translate(-(transform.position));
	}

	inline void SetAspectRatio(int width, int height)
	{
		aspectRatio = width / static_cast<float>(height);
	}
};