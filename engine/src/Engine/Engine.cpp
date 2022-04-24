#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/Instrumentation.hpp"

#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Memory/Memory.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/CameraParameters.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDeviceOpenGL.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/ModelManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/InputManager.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"

Engine::Engine(
	AllocatorManager* allocatorManager,
	kokko::Filesystem* filesystem,
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
		assetLoader, mainWindow.instance->GetInputManager(), resManagers);
}

Engine::~Engine()
{
	world.instance->Deinitialize();
	debug.instance->Deinitialize();

	world.Delete();
	materialManager.Delete();
	shaderManager.Delete();
	textureManager.Delete();
	meshManager.Delete();
	debug.Delete();
	systemAllocator->MakeDelete(renderDevice);
	systemAllocator->MakeDelete(time);
	mainWindow.Delete();
}

bool Engine::Initialize(const kokko::WindowSettings& windowSettings)
{
	KOKKO_PROFILE_FUNCTION();

	if (mainWindow.instance->Initialize(windowSettings) == false)
		return false;

	DebugLog* debugLog = debug.instance->GetLog();
	if (debugLog->OpenLogFile("log.txt", false) == false)
		return false;

	if (debug.instance->Initialize(mainWindow.instance, meshManager.instance,
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
	world.instance->Update();
}

void Engine::Render(const CameraParameters& editorCamera, const Framebuffer& framebuffer)
{
	KOKKO_PROFILE_FUNCTION();

	world.instance->Render(editorCamera, framebuffer);
	world.instance->DebugRender(debug.instance->GetVectorRenderer(), settings.renderDebug);

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
