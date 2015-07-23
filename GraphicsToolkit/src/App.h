#pragma once

#include "Time.h"
#include "Window.h"
#include "Camera.h"
#include "Renderer.h"
#include "ShaderProgram.h"

class App
{
private:
	static App* instance;
	
	Time time;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	
	RenderObjectId testCube;
	ShaderProgram simpleShader;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();
	
	static Renderer* GetRenderer();
};
