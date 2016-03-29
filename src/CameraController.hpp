#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"

struct Camera;
struct GLFWwindow;

class CameraController
{
private:
	Camera* controlledCamera = nullptr;

	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;

	Vec3f cameraVelocity;
	float cameraMaximumSpeed = 2.0f;

	bool mouseLookEnable = false;
	bool mouseGrabActive = false;

public:
	void SetControlledCamera(Camera* camera);
	void Update();
};
