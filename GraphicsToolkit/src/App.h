#pragma once

#include "Window.h"
#include "Renderer.h"

class App
{
private:
	Window mainWindow;
	Renderer renderer;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();
};
