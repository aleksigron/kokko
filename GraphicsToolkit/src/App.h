#pragma once

#include "Time.h"
#include "Window.h"
#include "Camera.h"
#include "Renderer.h"

class App
{
private:
	Time time;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();
};
