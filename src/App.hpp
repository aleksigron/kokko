#pragma once

#include "AppSettings.hpp"
#include "Rendering/Camera.hpp"
#include "CameraController.hpp"

class App
{
private:
	static App* instance;

	AppSettings settings;

	Camera mainCamera;
	CameraController cameraController;
	bool cameraControllerEnable;
	
public:
	App();
	~App();
	
	void Initialize();
	void Update();

	void SetCameraControllerEnable(bool enable)
	{
		cameraControllerEnable = enable;
	}

	AppSettings* GetSettings() { return &settings; }

	static App* GetInstance() { return App::instance; }
};
