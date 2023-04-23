#pragma once

#include "Core/Optional.hpp"

#include "Engine/EngineSettings.hpp"
#include "Engine/InstanceAllocatorPair.hpp"

class Allocator;
class AllocatorManager;
class Debug;
class Time;

struct CameraParameters;

namespace kokko
{
class AssetLoader;
class Filesystem;
class MaterialManager;
class MeshManager;
class ModelManager;
class ShaderManager;
class TextureManager;
class WindowManager;
class World;
struct ResourceManagers;
struct WindowSettings;

namespace render
{
struct CommandBuffer;
class CommandEncoder;
class CommandExecutor;
class Device;
class Framebuffer;
}
}

class Engine
{
private:
	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	kokko::EngineSettings settings;

	kokko::Filesystem* filesystem;
	kokko::AssetLoader* assetLoader;
    kokko::render::Device* renderDevice;
	kokko::render::CommandBuffer* commandBuffer;
	kokko::render::CommandEncoder* commandEncoder;
	kokko::render::CommandExecutor* commandExecutor;

	InstanceAllocatorPair<kokko::WindowManager> windowManager;
	Time* time;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<kokko::MeshManager> meshManager;
	InstanceAllocatorPair<kokko::ModelManager> modelManager;
	InstanceAllocatorPair<kokko::TextureManager> textureManager;
	InstanceAllocatorPair<kokko::ShaderManager> shaderManager;
	InstanceAllocatorPair<kokko::MaterialManager> materialManager;
	InstanceAllocatorPair<kokko::World> world;

public:
	Engine(
		AllocatorManager* allocatorManager,
		kokko::Filesystem* filesystem,
		kokko::AssetLoader* assetLoader);
	~Engine();

	bool Initialize(const kokko::WindowSettings& windowSettings);

	void StartFrame();
	void UpdateWorld();
	void Render(const Optional<CameraParameters>& editorCamera, const kokko::render::Framebuffer& framebuffer);
	void EndFrame();

	void SetAppPointer(void* app);

	kokko::EngineSettings* GetSettings() { return &settings; }
	kokko::WindowManager* GetWindowManager() { return windowManager.instance; }
	kokko::render::Device* GetRenderDevice() { return renderDevice; }
	kokko::render::CommandEncoder* GetCommandEncoder() { return commandEncoder; }
	Debug* GetDebug() { return debug.instance; }
	kokko::Filesystem* GetFilesystem() { return filesystem; }
	kokko::MeshManager* GetMeshManager() { return meshManager.instance; }
	kokko::ModelManager* GetModelManager() { return modelManager.instance; }
	kokko::ShaderManager* GetShaderManager() { return shaderManager.instance; }
	kokko::TextureManager* GetTextureManager() { return textureManager.instance; }
	kokko::MaterialManager* GetMaterialManager() { return materialManager.instance; }
	kokko::World* GetWorld() { return world.instance; }

	kokko::ResourceManagers GetResourceManagers();
};
