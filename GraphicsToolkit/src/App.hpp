#pragma once

#include "Time.hpp"
#include "Input.hpp"
#include "Window.hpp"
#include "Camera.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "CameraController.hpp"
#include "Scene.hpp"
#include "StackAllocator.hpp"

class App
{
private:
	static App* instance;
	
	Time time;
	Input input;
	Window mainWindow;
	Camera mainCamera;
	Renderer renderer;
	ResourceManager resourceManager;
	CameraController cameraController;
	Scene scene;

	StackAllocator stackAllocator;
	
public:
	App();
	~App();
	
	bool Initialize();
	bool HasRequestedQuit();
	
	void Update();

	inline static Input* GetInput()
	{ return &(App::instance->input); }

	inline static Window* GetMainWindow()
	{ return &(App::instance->mainWindow); }

	inline static Renderer* GetRenderer()
	{ return &(App::instance->renderer); }

	inline static ResourceManager* GetResourceManager()
	{ return &(App::instance->resourceManager); }

	inline static StackAllocator* GetStackAllocator()
	{ return &(App::instance->stackAllocator); }
};
