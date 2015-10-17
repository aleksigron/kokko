#pragma once

#include "Vec2.hpp"
#include "Vec3.hpp"

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
