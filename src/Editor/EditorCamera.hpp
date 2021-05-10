#pragma once

#include "Math/Vec3.hpp"
#include "Math/Projection.hpp"

struct Mat3x3f;
struct Mat4x4fBijection;

class InputManager;

class EditorCamera
{
private:
	InputManager* inputManager;

	ProjectionParameters projection;

	Vec3f cameraPosition;
	Vec3f cameraVelocity;
	float cameraYaw;
	float cameraPitch;

	bool mouseLookActive;
	bool mouseGrabActive;

	float cameraAimSensitivity;

	static Mat3x3f GetOrientation(float yaw, float pitch);

public:
	EditorCamera();

	void SetInputManager(InputManager* inputManager);
	void LookAt(const Vec3f& position, const Vec3f& lookAtTarget);

	void Update();

	/*
	* Forward is the camera transform (view space to world space)
	* Inverse is the view transform (world space to view space)
	*/
	Mat4x4fBijection GetCameraTransform() const;
	ProjectionParameters GetProjectionParameters() const;
};
