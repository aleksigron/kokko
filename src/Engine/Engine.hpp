#pragma once

class AllocatorManager;
class Window;
class Time;
class RenderDevice;
class EntityManager;
class Renderer;
class MeshManager;
class ShaderManager;
class MaterialManager;
class ResourceManager;
class SceneManager;
class LightManager;

class Debug;

class Engine
{
private:
	static Engine* instance;

	AllocatorManager* allocatorManager;

	Window* mainWindow;
	Time* time;
	RenderDevice* renderDevice;
	EntityManager* entityManager;
	LightManager* lightManager;
	Renderer* renderer;
	MeshManager* meshManager;
	ShaderManager* shaderManager;
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

	AllocatorManager* GetAllocatorManager() { return allocatorManager; }

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
