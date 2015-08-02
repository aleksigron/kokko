#pragma once

#include "Time.h"
#include "Window.h"
#include "Camera.h"
#include "Renderer.h"
#include "ResourceManager.h"

class App
{
private:
	static App* instance;
	
	Time time;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	ResourceManager resourceManager;
	
	ObjectId cube0;
	ObjectId cube1;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();

	inline static Renderer* GetRenderer()
	{ return &(App::instance->renderer); }

	inline static ResourceManager* GetResourceManager()
	{ return &(App::instance->resourceManager); }
};
