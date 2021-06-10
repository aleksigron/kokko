#pragma once

#include "Engine/InstanceAllocatorPair.hpp"

class Allocator;
class AllocatorManager;
class Window;
class EditorUI;
class Time;
class RenderDevice;
class World;
class EntityManager;
class Scene;
class Renderer;
class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;
class LightManager;
class CameraSystem;
class TerrainManager;
class ParticleSystem;
class EnvironmentManager;
class ScriptSystem;

class Debug;

class Engine
{
private:
	AllocatorManager* allocatorManager;

	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	InstanceAllocatorPair<Window> mainWindow;
	InstanceAllocatorPair<EditorUI> editorUI;
	Time* time;
	RenderDevice* renderDevice;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<MeshManager> meshManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
	InstanceAllocatorPair<TerrainManager> terrainManager;
	InstanceAllocatorPair<ParticleSystem> particleSystem;
	InstanceAllocatorPair<EnvironmentManager> environmentManager;
	InstanceAllocatorPair<World> world;


public:
	Engine();
	~Engine();

	bool Initialize();
	void FrameStart();
	void Update();

	void SetAppPointer(void* app);

	AllocatorManager* GetAllocatorManager() { return allocatorManager; }
	Window* GetMainWindow() { return mainWindow.instance; }
	MeshManager* GetMeshManager() { return meshManager.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	EnvironmentManager* GetEnvironmentManager() { return environmentManager.instance; }
};
