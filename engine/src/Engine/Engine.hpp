#pragma once

#include "Engine/EngineSettings.hpp"
#include "Engine/InstanceAllocatorPair.hpp"

class Allocator;
class AllocatorManager;
class Debug;
class Filesystem;
class Framebuffer;
class RenderDevice;
class Time;
class Window;

class World;
class MeshManager;
class ShaderManager;
class MaterialManager;
class TextureManager;

struct CameraParameters;

namespace kokko
{
class AssetLoader;
class ModelManager;
}

class Engine
{
private:
	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	EngineSettings settings;

	Filesystem* filesystem;
	kokko::AssetLoader* assetLoader;

	InstanceAllocatorPair<Window> mainWindow;
	Time* time;
	RenderDevice* renderDevice;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<MeshManager> meshManager;
	InstanceAllocatorPair<kokko::ModelManager> modelManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
	InstanceAllocatorPair<World> world;

public:
	Engine(
		AllocatorManager* allocatorManager,
		Filesystem* filesystem,
		kokko::AssetLoader* assetLoader);
	~Engine();

	bool Initialize();

	void StartFrame();
	void UpdateWorld();
	void Render(const CameraParameters& editorCamera, const Framebuffer& framebuffer);
	void EndFrame();

	void SetAppPointer(void* app);

	EngineSettings* GetSettings() { return &settings; }
	Window* GetMainWindow() { return mainWindow.instance; }
	RenderDevice* GetRenderDevice() { return renderDevice; }
	Debug* GetDebug() { return debug.instance; }
	Filesystem* GetFilesystem() { return filesystem; }
	MeshManager* GetMeshManager() { return meshManager.instance; }
	kokko::ModelManager* GetModelManager() { return modelManager.instance; }
	ShaderManager* GetShaderManager() { return shaderManager.instance; }
	TextureManager* GetTextureManager() { return textureManager.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	World* GetWorld() { return world.instance; }
};
