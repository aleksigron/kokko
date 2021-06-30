#pragma once

#include "Engine/InstanceAllocatorPair.hpp"

class Allocator;
class AllocatorManager;
class Window;
class EditorUI;
class Time;
class RenderDevice;
class World;
class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;
class EnvironmentManager;

class Debug;

class Engine
{
private:
	AllocatorManager* allocatorManager;

	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	InstanceAllocatorPair<Window> mainWindow;
	Time* time;
	RenderDevice* renderDevice;
	InstanceAllocatorPair<EditorUI> editorUI;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<MeshManager> meshManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
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
