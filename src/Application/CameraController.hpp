#pragma once

#include "Math/Vec3.hpp"

class App;
class Camera;
class SceneManager;
class Window;

class CameraController
{
private:
	App* app;
	SceneManager* sceneManager;
	Window* window;
	Camera* controlledCamera;

	float cameraYaw = 0.0f;
	float cameraPitch = 0.0f;
	
	Vec3f cameraVelocity;
	float cameraSpeed = 4.0f;

	bool mouseLookActive = false;
	bool mouseGrabActive = false;

	float cameraAimSensitivity = -1.0f;

	void VerifySensitityIsLoaded();

public:
	CameraController(App* app, SceneManager* sceneManager, Window* window);
	~CameraController();

	void SetControlledCamera(Camera* camera);
	
	void Update();
};
