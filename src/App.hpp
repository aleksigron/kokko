#pragma once

#include "AppSettings.hpp"
#include "Camera.hpp"
#include "CameraController.hpp"

class App
{
private:
	static App* instance;

	AppSettings settings;

	Camera mainCamera;
	CameraController cameraController;
	
public:
	App();
	~App();
	
	void Initialize();
	void Update();

	AppSettings* GetSettings() { return &settings; }

	static App* GetInstance() { return App::instance; }
};
