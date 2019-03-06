#pragma once

#include "Mat4x4.hpp"
#include "Entity.hpp"

class Scene;

class Camera
{
public:
	enum class Projection
	{
		Perspective,
		Orthographic
	};

private:
	Entity entity;

public:
	Camera();

	// The camera's vertical field of view in radians
	float perspectiveFieldOfView = 1.0f;

	// The height of the camera's orthogonal viewport
	float orthogonalHeight = 1.0f;

	// The ratio of the viewport's width to its height (x / y)
	float aspectRatio = 1.0f;

	float nearClipDistance = 0.1f;
	float farClipDistance = 100.0f;
	
	Projection projectionType = Projection::Perspective;

	Entity GetEntity() const { return entity; }
	void SetEntity(Entity e) { entity = e; }

	static Mat4x4f GetViewMatrix(const Mat4x4f& cameraTransform);
	Mat4x4f GetProjectionMatrix() const;

	void SetAspectRatio(float width, float height)
	{
		aspectRatio = width / height;
	}
};
