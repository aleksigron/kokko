#pragma once

#include "Mat4x4.hpp"
#include "Vec2.hpp"

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
	unsigned int sceneId;
	unsigned int sceneObjectId;

public:
	Camera();
	~Camera();

	void InitializeSceneObject(Scene* scene);

	// The camera's vertical field of view in radians
	float perspectiveFieldOfView = 1.0f;

	// The height of the camera's orthogonal viewport
	float orthogonalHeight = 1.0f;

	// The ratio of the viewport's width to its height (x / y)
	float aspectRatio = 1.0f;

	float nearClipDistance = 0.1f;
	float farClipDistance = 100.0f;
	
	Projection projectionType = Projection::Perspective;

	unsigned int GetContainingSceneId() const { return sceneId; }
	unsigned int GetSceneObjectId() const { return sceneObjectId; }

	Mat4x4f GetViewMatrix() const;
	Mat4x4f GetProjectionMatrix() const;

	void SetAspectRatio(float width, float height)
	{
		aspectRatio = width / height;
	}
};
