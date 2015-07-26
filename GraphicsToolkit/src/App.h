#pragma once

#include "Time.h"
#include "Window.h"
#include "Camera.h"
#include "Renderer.h"
#include "ShaderManager.h"

class App
{
private:
	static App* instance;
	
	Time time;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	ShaderManager shaderManager;
	
	RenderObjectId testCube;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();

	inline static Renderer* GetRenderer()
	{ return &(App::instance->renderer); }

	inline static ShaderManager* GetShaderManager()
	{ return &(App::instance->shaderManager); }
};
