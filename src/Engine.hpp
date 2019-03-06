#pragma once

class Window;
class Time;
class EntityManager;
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
	EntityManager* entityManager;
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
	EntityManager* GetEntityManager() { return entityManager; }
	Renderer* GetRenderer() { return renderer; }
	ResourceManager* GetResourceManager() { return resourceManager; }
	SceneManager* GetSceneManager() { return sceneManager; }

	Debug* GetDebug() { return debug; }
};
