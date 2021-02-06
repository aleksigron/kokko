#pragma once

#include "Math/Vec2.hpp"
#include "Math/Vec3.hpp"

struct GLFWwindow;

class Camera;

class CameraController
{
private:
	Camera* controlledCamera = nullptr;

	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;
	
	Vec3f cameraVelocity;
	float cameraSpeed = 4.0f;

	bool mouseLookActive = false;
	bool mouseGrabActive = false;

	float cameraAimSensitivity = -1.0f;

	void VerifySensitityIsLoaded();

public:
	CameraController();
	~CameraController();

	void SetControlledCamera(Camera* camera);
	
	void Update();
};
