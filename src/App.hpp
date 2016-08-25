#pragma once

#include "Camera.hpp"
#include "CameraController.hpp"

class World;

class Debug;

class App
{
private:
	static App* instance;

	Camera mainCamera;
	CameraController cameraController;
	
public:
	App();
	~App();
	
	void Initialize();
	void Update();

	static App* GetInstance() { return App::instance; }
};
