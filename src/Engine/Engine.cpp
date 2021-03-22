#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/ParticleSystem.hpp"
#include "Graphics/TerrainManager.hpp"

#include "Memory/AllocatorManager.hpp"
#include "Memory/Memory.hpp"
#include "Memory/ProxyAllocator.hpp"

#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDeviceOpenGL.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scene/SceneManager.hpp"
#include "Scene/Scene.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/Time.hpp"
#include "System/Window.hpp"

template <typename Type>
void Engine::InstanceAllocatorPair<Type>::CreateScope(AllocatorManager* manager, const char* name, Allocator* alloc)
{
	allocator = manager->CreateAllocatorScope(name, alloc);
}

Engine::Engine()
{
	Memory::InitializeMemorySystem();
	Allocator* alloc = Memory::GetDefaultAllocator();

	allocatorManager = alloc->MakeNew<AllocatorManager>(alloc);

	mainWindow.CreateScope(allocatorManager, "Window", alloc);
	mainWindow.New(mainWindow.allocator);

	systemAllocator = allocatorManager->CreateAllocatorScope("System", alloc);
	time = systemAllocator->MakeNew<Time>();
	renderDevice = systemAllocator->MakeNew<RenderDeviceOpenGL>();

	debug.CreateScope(allocatorManager, "Debug", alloc);
	debug.New(debug.allocator, allocatorManager, mainWindow.instance, renderDevice);

	entityManager.CreateScope(allocatorManager, "EntityManager", alloc);
	entityManager.New(entityManager.allocator);

	meshManager.CreateScope(allocatorManager, "MeshManager", alloc);
	meshManager.New(meshManager.allocator, renderDevice);

	textureManager.CreateScope(allocatorManager, "TextureManager", alloc);
	textureManager.New(textureManager.allocator, renderDevice);

	shaderManager.CreateScope(allocatorManager, "ShaderManager", alloc);
	shaderManager.New(shaderManager.allocator, renderDevice);

	materialManager.CreateScope(allocatorManager, "MaterialManager", alloc);
	materialManager.New(materialManager.allocator, renderDevice, shaderManager.instance, textureManager.instance);

	lightManager.CreateScope(allocatorManager, "LightManager", alloc);
	lightManager.New(lightManager.allocator);

	sceneManager.CreateScope(allocatorManager, "SceneManager", alloc);
	sceneManager.New(this, sceneManager.allocator);

	terrainManager.CreateScope(allocatorManager, "TerrainManager", alloc);
	terrainManager.New(terrainManager.allocator, renderDevice, meshManager.instance, materialManager.instance);

	particleSystem.CreateScope(allocatorManager, "ParticleEffects", alloc);
	particleSystem.New(particleSystem.allocator, renderDevice, shaderManager.instance, meshManager.instance);

	environmentManager.CreateScope(allocatorManager, "EnvironmentManager", alloc);
	environmentManager.New(environmentManager.allocator, renderDevice,
		shaderManager.instance, meshManager.instance, textureManager.instance);

	renderer.CreateScope(allocatorManager, "Renderer", alloc);
	renderer.New(renderer.allocator, renderDevice, lightManager.instance,
		shaderManager.instance, meshManager.instance, materialManager.instance, textureManager.instance);

	scriptSystem.CreateScope(allocatorManager, "ScriptSystem", alloc);
	scriptSystem.New(this, scriptSystem.allocator);
}

Engine::~Engine()
{
	renderer.instance->Deinitialize();
	debug.instance->Deinitialize();

	scriptSystem.Delete();
	renderer.Delete();
	environmentManager.Delete();
	particleSystem.Delete();
	terrainManager.Delete();
	sceneManager.Delete();
	lightManager.Delete();
	materialManager.Delete();
	shaderManager.Delete();
	textureManager.Delete();
	meshManager.Delete();
	entityManager.Delete();
	debug.Delete();
	systemAllocator->MakeDelete(this->time);
	systemAllocator->MakeDelete(this->renderDevice);
	mainWindow.Delete();

	Allocator* defaultAllocator = Memory::GetDefaultAllocator();
	defaultAllocator->MakeDelete(this->allocatorManager);

	Memory::DeinitializeMemorySystem();
}

bool Engine::Initialize()
{
	Vec2i windowSize(1920, 1080);

	if (mainWindow.instance->Initialize(windowSize.x, windowSize.y, "Kokko"))
	{
		const char* const logFilename = "log.txt";
		const char* const debugFontFilename = "res/fonts/gohufont-uni-14.bdf";

		DebugLog* debugLog = debug.instance->GetLog();
		debugLog->OpenLogFile(logFilename, false);

		DebugTextRenderer* debugTextRenderer = debug.instance->GetTextRenderer();
		bool fontLoaded = debugTextRenderer->LoadBitmapFont(textureManager.instance, debugFontFilename);
		if (fontLoaded == false)
		{
			Allocator* defaultAllocator = Memory::GetDefaultAllocator();
			String logText = String(defaultAllocator, "Loading font at ") + debugFontFilename + " failed.";
			debugLog->Log(logText);
		}

		debug.instance->Initialize(mainWindow.instance, renderer.instance,
			meshManager.instance, shaderManager.instance, sceneManager.instance);

		textureManager.instance->Initialize();
		renderer.instance->Initialize(mainWindow.instance, entityManager.instance, environmentManager.instance);
		terrainManager.instance->Initialize(renderer.instance, shaderManager.instance);
		particleSystem.instance->Initialize(renderer.instance);

		return true;
	}
	else
	{
		return false;
	}
}

void Engine::Update()
{
	this->time->Update();

	scriptSystem.instance->UpdateScripts();

	unsigned int primarySceneId = sceneManager.instance->GetPrimarySceneId();
	Scene* primaryScene = sceneManager.instance->GetScene(primarySceneId);

	// Propagate transform updates from Scene to other systems that require it
	ITransformUpdateReceiver* transformUpdateReceivers[] = { lightManager.instance, renderer.instance };
	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	primaryScene->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	renderer.instance->Render(primaryScene);

	debug.instance->Render(primaryScene);

	mainWindow.instance->UpdateInput();
	mainWindow.instance->Swap();
}

void Engine::SetAppPointer(void* app)
{
	scriptSystem.instance->SetAppPointer(app);
}
