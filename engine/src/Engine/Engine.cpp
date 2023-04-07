#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/Instrumentation.hpp"

#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Memory/RootAllocator.hpp"

#include "Platform/Window.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/CameraParameters.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderCommandBuffer.hpp"
#include "Rendering/RenderCommandEncoder.hpp"
#include "Rendering/RenderDevice.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/InputManager.hpp"
#include "System/Time.hpp"
#include "System/WindowManager.hpp"

Engine::Engine(
	AllocatorManager* allocatorManager,
	kokko::Filesystem* filesystem,
	kokko::AssetLoader* assetLoader) :
	filesystem(filesystem),
	assetLoader(assetLoader)
{
	KOKKO_PROFILE_FUNCTION();

	Allocator* alloc = RootAllocator::GetDefaultAllocator();
    systemAllocator = allocatorManager->CreateAllocatorScope("System", alloc);

    renderDevice = RenderDevice::Create(systemAllocator);
	commandBuffer = systemAllocator->MakeNew<kokko::render::CommandBuffer>(systemAllocator);
	commandEncoder = systemAllocator->MakeNew<kokko::render::CommandEncoder>(systemAllocator, commandBuffer);
    
    windowManager.CreateScope(allocatorManager, "Window", alloc);
    windowManager.New(windowManager.allocator);

    kokko::Window* mainWindow = windowManager.instance->GetWindow();

	time = systemAllocator->MakeNew<Time>();

	debug.CreateScope(allocatorManager, "Debug", alloc);
	debug.New(debug.allocator, allocatorManager, renderDevice, filesystem);

	debugNameAllocator = allocatorManager->CreateAllocatorScope("EntityDebugNames", alloc);

	meshManager.CreateScope(allocatorManager, "MeshManager", alloc);
	meshManager.New(meshManager.allocator, assetLoader, renderDevice);

	modelManager.CreateScope(allocatorManager, "ModelManager", alloc);
	modelManager.New(modelManager.allocator, assetLoader, meshManager.instance);

	textureManager.CreateScope(allocatorManager, "TextureManager", alloc);
	textureManager.New(textureManager.allocator, assetLoader, renderDevice);

	shaderManager.CreateScope(allocatorManager, "ShaderManager", alloc);
	shaderManager.New(shaderManager.allocator, filesystem, assetLoader, renderDevice);

	materialManager.CreateScope(allocatorManager, "MaterialManager", alloc);
	materialManager.New(materialManager.allocator, assetLoader, renderDevice,
		shaderManager.instance, textureManager.instance);

	kokko::ResourceManagers resManagers = GetResourceManagers();

	world.CreateScope(allocatorManager, "World", alloc);
	world.New(allocatorManager, world.allocator, debugNameAllocator, renderDevice,
		assetLoader, resManagers);
}

Engine::~Engine()
{
	world.instance->Deinitialize();
	debug.instance->Deinitialize();

	world.Delete();
	materialManager.Delete();
	shaderManager.Delete();
	textureManager.Delete();
	modelManager.Delete();
	meshManager.Delete();
	debug.Delete();
	systemAllocator->MakeDelete(time);
	windowManager.Delete();

	systemAllocator->MakeDelete(commandEncoder);
    systemAllocator->MakeDelete(renderDevice);
}

bool Engine::Initialize(const kokko::WindowSettings& windowSettings)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowManager.instance->Initialize(windowSettings, renderDevice->GetNativeDevice()) == false)
		return false;

	if (debug.instance->Initialize(windowManager.instance->GetWindow(), meshManager.instance,
		shaderManager.instance, textureManager.instance) == false)
		return false;

	textureManager.instance->Initialize();

	world.instance->Initialize();

	return true;
}

void Engine::StartFrame()
{
	if (debug.instance->ShouldBeginProfileSession())
		Instrumentation::Get().BeginSession("runtime_trace.json");

	if (debug.instance->ShouldEndProfileSession())
		Instrumentation::Get().EndSession();
}

void Engine::UpdateWorld()
{
	KOKKO_PROFILE_FUNCTION();

	time->Update();
	world.instance->Update(windowManager.instance->GetWindow()->GetInputManager());
}

void Engine::Render(const Optional<CameraParameters>& editorCamera, const Framebuffer& framebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	world.instance->Render(windowManager.instance->GetWindow(), editorCamera, framebuffer);
	world.instance->DebugRender(debug.instance->GetVectorRenderer(), settings.renderDebug);

	if (settings.enableDebugTools)
	{
		debug.instance->Render(commandEncoder, world.instance, framebuffer, editorCamera);
	}
}

void Engine::EndFrame()
{
    kokko::Window* window = windowManager.instance->GetWindow();
    window->Swap();
    windowManager.instance->ProcessEvents();
    window->UpdateInput();

    window->SetSwapInterval(settings.verticalSync ? 1 : 0);
}

void Engine::SetAppPointer(void* app)
{
	world.instance->GetScriptSystem()->SetAppPointer(app);
}

kokko::ResourceManagers Engine::GetResourceManagers()
{
	kokko::ResourceManagers resManagers;
	resManagers.meshManager = meshManager.instance;
	resManagers.modelManager = modelManager.instance;
	resManagers.shaderManager = shaderManager.instance;
	resManagers.materialManager = materialManager.instance;
	resManagers.textureManager = textureManager.instance;
	return resManagers;
}
