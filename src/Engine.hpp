#pragma once

class Window;
class Time;
class Scene;
class World;
class Renderer;
class ResourceManager;

class Debug;

class Engine
{
private:
	static Engine* instance;

	Window* mainWindow;
	Time* time;
	Scene* scene;
	World* world;
	Renderer* renderer;
	ResourceManager* resourceManager;

	Debug* debug;

public:
	Engine();
	~Engine();

	bool Initialize();
	void Update();

	static Engine* GetInstance() { return instance; }

	Window* GetMainWindow() { return mainWindow; }
	Time* GetTime() { return time; }
	Scene* GetScene() { return scene; }
	World* GetWorld() { return world; }
	Renderer* GetRenderer() { return renderer; }
	ResourceManager* GetResourceManager() { return resourceManager; }

	Debug* GetDebug() { return debug; }
};
