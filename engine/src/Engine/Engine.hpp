#pragma once

#include "Core/Optional.hpp"
#include "Core/UniquePtr.hpp"

#include "Engine/EngineSettings.hpp"
#include "Engine/InstanceAllocatorPair.hpp"

namespace kokko
{

class Allocator;
class AllocatorManager;
class AssetLoader;
class Debug;
class Filesystem;
class MaterialManager;
class MeshManager;
class ModelManager;
class ShaderManager;
class TextureManager;
class Time;
class WindowManager;
class World;

struct CameraParameters;
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

class Engine
{
private:
	Allocator* systemAllocator;
	Allocator* debugNameAllocator;

	EngineSettings settings;

	Filesystem* filesystem;
	AssetLoader* assetLoader;
	render::Device* renderDevice;
	UniquePtr<render::CommandBuffer> commandBuffer;
	UniquePtr<render::CommandEncoder> commandEncoder;
	render::CommandExecutor* commandExecutor;

	InstanceAllocatorPair<WindowManager> windowManager;
	UniquePtr<Time> engineTime;
	InstanceAllocatorPair<Debug> debug;
	InstanceAllocatorPair<ModelManager> modelManager;
	InstanceAllocatorPair<TextureManager> textureManager;
	InstanceAllocatorPair<ShaderManager> shaderManager;
	InstanceAllocatorPair<MaterialManager> materialManager;
	InstanceAllocatorPair<World> world;

public:
	Engine(
		AllocatorManager* allocatorManager,
		Filesystem* filesystem,
		AssetLoader* assetLoader);
	~Engine();

	bool Initialize(const WindowSettings& windowSettings);

	void StartFrame();
	void Update();
	void Render(const Optional<CameraParameters>& editorCamera, const render::Framebuffer& framebuffer);
	void EndFrame();

	void SetAppPointer(void* app);

	EngineSettings* GetSettings() { return &settings; }
	WindowManager* GetWindowManager() { return windowManager.instance; }
	render::Device* GetRenderDevice() { return renderDevice; }
	render::CommandEncoder* GetCommandEncoder() { return commandEncoder.Get(); }
	Debug* GetDebug() { return debug.instance; }
	Filesystem* GetFilesystem() { return filesystem; }
	ModelManager* GetModelManager() { return modelManager.instance; }
	ShaderManager* GetShaderManager() { return shaderManager.instance; }
	TextureManager* GetTextureManager() { return textureManager.instance; }
	MaterialManager* GetMaterialManager() { return materialManager.instance; }
	World* GetWorld() { return world.instance; }

	ResourceManagers GetResourceManagers();
};

} // namespace kokko
