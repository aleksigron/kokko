#pragma once

#include "Window.h"
#include "Camera.h"
#include "Renderer.h"

class App
{
private:
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
