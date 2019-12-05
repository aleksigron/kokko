#pragma once

class Window;
class Time;
class EntityManager;
class Renderer;
class MeshManager;
class MaterialManager;
class ResourceManager;
class SceneManager;
class LightManager;

class Debug;

class Engine
{
private:
	static Engine* instance;

	Window* mainWindow;
	Time* time;
	EntityManager* entityManager;
	LightManager* lightManager;
	Renderer* renderer;
	MeshManager* meshManager;
	MaterialManager* materialManager;
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
	LightManager* GetLightManager() { return lightManager; }
	Renderer* GetRenderer() { return renderer; }
	MaterialManager* GetMaterialManager() { return materialManager; }
	MeshManager* GetMeshManager() { return meshManager; }
	ResourceManager* GetResourceManager() { return resourceManager; }
	SceneManager* GetSceneManager() { return sceneManager; }

	Debug* GetDebug() { return debug; }
};
