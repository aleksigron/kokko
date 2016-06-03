#pragma once

#include "Time.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "CameraController.hpp"
#include "Scene.hpp"

#include "DebugTextRenderer.hpp"

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
	Scene scene;
	DebugTextRenderer debugTextRenderer;
	
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

	static DebugTextRenderer* GetDebugTextRenderer()
	{ return &(App::instance->debugTextRenderer); }
};
