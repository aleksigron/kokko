#pragma once

#include "Engine/EngineSettings.hpp"
#include "Engine/InstanceAllocatorPair.hpp"

class Allocator;
class AllocatorManager;
class Debug;
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
class Filesystem;
class ModelManager;
struct ResourceManagers;
struct WindowSettings;
}

class Engine
{
private:
	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	kokko::EngineSettings settings;

	kokko::Filesystem* filesystem;
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
		kokko::Filesystem* filesystem,
		kokko::AssetLoader* assetLoader);
	~Engine();

	bool Initialize(const kokko::WindowSettings& windowSettings);

	void StartFrame();
	void UpdateWorld();
	void Render(const CameraParameters& editorCamera, const Framebuffer& framebuffer);
	void EndFrame();

	void SetAppPointer(void* app);

	kokko::EngineSettings* GetSettings() { return &settings; }
	Window* GetMainWindow() { return mainWindow.instance; }
	RenderDevice* GetRenderDevice() { return renderDevice; }
	Debug* GetDebug() { return debug.instance; }
	kokko::Filesystem* GetFilesystem() { return filesystem; }
	MeshManager* GetMeshManager() { return meshManager.instance; }
	kokko::ModelManager* GetModelManager() { return modelManager.instance; }
	ShaderManager* GetShaderManager() { return shaderManager.instance; }
	TextureManager* GetTextureManager() { return textureManager.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	World* GetWorld() { return world.instance; }

	kokko::ResourceManagers GetResourceManagers();
};
