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

	Vec2d prevCursorPos;
	bool mouseControlEnable = false;

	static void KeyCallback(GLFWwindow* window, int key, int code, int act, int mods);

public:
	void SetControlledCamera(Camera* camera);
	void Update();
};
