#pragma once

#include "Window.h"

class App
{
private:
	Window mainWindow;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();
};
