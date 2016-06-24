#pragma once

#include "Time.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "CameraController.hpp"
#include "Scene.hpp"

#include "DebugTextRenderer.hpp"

class World;

class Debug;

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

	World* world;

	Debug* debug;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();

	static App* GetInstance() { return App::instance; }

	inline static Window* GetMainWindow()
	{ return &(App::instance->mainWindow); }

	inline static Renderer* GetRenderer()
	{ return &(App::instance->renderer); }

	inline static ResourceManager* GetResourceManager()
	{ return &(App::instance->resourceManager); }

	Debug* GetDebug() { return debug; }
};
