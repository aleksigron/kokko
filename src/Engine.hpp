#pragma once

class Window;
class Time;
class Renderer;
class ResourceManager;
class SceneManager;

class Debug;

class Engine
{
private:
	static Engine* instance;

	Window* mainWindow;
	Time* time;
	Renderer* renderer;
	ResourceManager* resourceManager;
	SceneManager* sceneManager;

	Debug* debug;

public:
	Engine();
	~Engine();

	bool Initialize();
	void Update();

	static Engine* GetInstance() { return instance; }

	Window* GetMainWindow() { return mainWindow; }
	Renderer* GetRenderer() { return renderer; }
	ResourceManager* GetResourceManager() { return resourceManager; }
	SceneManager* GetSceneManager() { return sceneManager; }

	Debug* GetDebug() { return debug; }
};
