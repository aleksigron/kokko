#pragma once

#include "Time.h"
#include "Window.h"
#include "Camera.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "CameraController.h"

class App
{
private:
	static App* instance;
	
	Time time;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	ResourceManager resourceManager;
	CameraController cameraController;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();

	inline static Window* GetMainWindow()
	{ return &(App::instance->mainWindow); }

	inline static Renderer* GetRenderer()
	{ return &(App::instance->renderer); }

	inline static ResourceManager* GetResourceManager()
	{ return &(App::instance->resourceManager); }
};
