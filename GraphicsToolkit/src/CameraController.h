#pragma once

#include "Vec2.h"
#include "Vec3.h"

struct Camera;
struct GLFWwindow;

class CameraController
{
private:
	Camera* controlledCamera = nullptr;

	Vec3f cameraVelocity;
	float cameraMaximumSpeed = 3.0f;

	bool mouseLookEnable = false;
	bool mouseGrabActive = false;

public:
	void SetControlledCamera(Camera* camera);
	void Update();
};
