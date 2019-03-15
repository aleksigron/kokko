#pragma once

class Window;
class Time;
class EntityManager;
class Renderer;
class MeshManager;
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
	MeshManager* meshManager;
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
	MeshManager* GetMeshManager() { return meshManager; }
	ResourceManager* GetResourceManager() { return resourceManager; }
	SceneManager* GetSceneManager() { return sceneManager; }

	Debug* GetDebug() { return debug; }
};
