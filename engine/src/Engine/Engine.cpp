#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/Instrumentation.hpp"

#include "Engine/World.hpp"

#include "Engine/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/Scene.hpp"

#include "Memory/AllocatorManager.hpp"
#include "Memory/Memory.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDeviceOpenGL.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/FilesystemDefault.hpp"
#include "System/InputManager.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"

Engine::Engine(
	AllocatorManager* allocatorManager,
	Filesystem* filesystem,
	kokko::AssetLoader* assetLoader) :
	filesystem(filesystem),
	assetLoader(assetLoader)
{
	KOKKO_PROFILE_FUNCTION();

	Allocator* alloc = Memory::GetDefaultAllocator();

	mainWindow.CreateScope(allocatorManager, "Window", alloc);
	mainWindow.New(mainWindow.allocator);

	systemAllocator = allocatorManager->CreateAllocatorScope("System", alloc);
	time = systemAllocator->MakeNew<Time>();
	renderDevice = systemAllocator->MakeNew<RenderDeviceOpenGL>();

	debug.CreateScope(allocatorManager, "Debug", alloc);
	debug.New(debug.allocator, allocatorManager, mainWindow.instance, renderDevice, filesystem);

	debugNameAllocator = allocatorManager->CreateAllocatorScope("EntityDebugNames", alloc);

	meshManager.CreateScope(allocatorManager, "MeshManager", alloc);
	meshManager.New(meshManager.allocator, filesystem, renderDevice);

	textureManager.CreateScope(allocatorManager, "TextureManager", alloc);
	textureManager.New(textureManager.allocator, assetLoader, renderDevice);

	shaderManager.CreateScope(allocatorManager, "ShaderManager", alloc);
	shaderManager.New(shaderManager.allocator, filesystem, assetLoader, renderDevice);

	materialManager.CreateScope(allocatorManager, "MaterialManager", alloc);
	materialManager.New(materialManager.allocator, assetLoader, renderDevice,
		shaderManager.instance, textureManager.instance);

	environmentManager.CreateScope(allocatorManager, "EnvironmentManager", alloc);
	environmentManager.New(environmentManager.allocator, filesystem, renderDevice,
		shaderManager.instance, meshManager.instance, textureManager.instance);

	ResourceManagers resManagers;
	resManagers.meshManager = meshManager.instance;
	resManagers.shaderManager = shaderManager.instance;
	resManagers.materialManager = materialManager.instance;
	resManagers.textureManager = textureManager.instance;
	resManagers.environmentManager = environmentManager.instance;

	world.CreateScope(allocatorManager, "World", alloc);
	world.New(allocatorManager, world.allocator, debugNameAllocator, renderDevice,
		filesystem, mainWindow.instance->GetInputManager(), resManagers);
}

Engine::~Engine()
{
	world.instance->Deinitialize();
	debug.instance->Deinitialize();

	world.Delete();
	environmentManager.Delete();
	materialManager.Delete();
	shaderManager.Delete();
	textureManager.Delete();
	meshManager.Delete();
	debug.Delete();
	systemAllocator->MakeDelete(renderDevice);
	systemAllocator->MakeDelete(time);
	mainWindow.Delete();
}

bool Engine::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	Vec2i windowSize(1920, 1080);

	if (mainWindow.instance->Initialize(windowSize.x, windowSize.y, "Kokko"))
	{
		DebugLog* debugLog = debug.instance->GetLog();
		debugLog->OpenLogFile("log.txt", false);

		debug.instance->Initialize(mainWindow.instance, meshManager.instance,
			shaderManager.instance, textureManager.instance);

		textureManager.instance->Initialize();
		environmentManager.instance->Initialize();

		world.instance->Initialize();

		return true;
	}
	else
	{
		return false;
	}
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
	world.instance->Update();
}

void Engine::Render(const CameraParameters& editorCamera, const Framebuffer& framebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	world.instance->Render(editorCamera, framebuffer);
	world.instance->DebugRender(&settings, debug.instance->GetVectorRenderer());

	debug.instance->Render(world.instance, framebuffer, editorCamera);
}

void Engine::EndFrame()
{
	mainWindow.instance->Swap();
	mainWindow.instance->ProcessEvents();
	mainWindow.instance->UpdateInput();

	mainWindow.instance->SetSwapInterval(settings.verticalSync ? 1 : 0);
}

void Engine::SetAppPointer(void* app)
{
	world.instance->GetScriptSystem()->SetAppPointer(app);
}
