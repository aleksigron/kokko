#pragma once

#include "Application/AppSettings.hpp"
#include "Application/CameraController.hpp"
#include "Rendering/Camera.hpp"

class Allocator;
class Engine;

class App
{
private:
	Engine* engine;
	Allocator* allocator;
	AppSettings settings;

	Camera mainCamera;
	CameraController cameraController;
	bool cameraControllerEnable;
	
public:
	App(Engine* engine, Allocator* allocator);
	~App();
	
	void Initialize();
	void Update();

	void SetCameraControllerEnable(bool enable)
	{
		cameraControllerEnable = enable;
	}

	AppSettings* GetSettings() { return &settings; }
};
