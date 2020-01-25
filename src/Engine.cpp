#include "Engine.hpp"

#include <cstdio>

#include "Memory/AllocatorManager.hpp"
#include "Memory/Memory.hpp"
#include "Memory/ProxyAllocator.hpp"

#include "System/Window.hpp"
#include "System/Time.hpp"
#include "EntityManager.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"
#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/ResourceManager.hpp"
#include "Resources/SceneManager.hpp"
#include "Resources/Scene.hpp"
#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"

#include "System/IncludeGLFW.hpp"

Engine* Engine::instance = nullptr;

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

Engine::Engine()
{
	Engine::instance = this;

	Memory::InitializeMemorySystem();
	Allocator* defaultAllocator = Memory::GetDefaultAllocator();

	this->allocatorManager = defaultAllocator->MakeNew<AllocatorManager>(defaultAllocator);

	this->mainWindow = defaultAllocator->MakeNew<Window>();
	this->debug = defaultAllocator->MakeNew<Debug>();
	this->time = defaultAllocator->MakeNew<Time>();
	this->entityManager = defaultAllocator->MakeNew<EntityManager>();

	Allocator* meshManagerAlloc = allocatorManager->CreateAllocatorScope("MeshManager", defaultAllocator);
	this->meshManager = defaultAllocator->MakeNew<MeshManager>(meshManagerAlloc);

	Allocator* materialManagerAlloc = allocatorManager->CreateAllocatorScope("MaterialManager", defaultAllocator);
	this->materialManager = defaultAllocator->MakeNew<MaterialManager>(materialManagerAlloc);

	this->resourceManager = defaultAllocator->MakeNew<ResourceManager>();

	Allocator* lightManagerAlloc = allocatorManager->CreateAllocatorScope("LightManager", defaultAllocator);
	this->lightManager = defaultAllocator->MakeNew<LightManager>(lightManagerAlloc);

	Allocator* rendererAlloc = allocatorManager->CreateAllocatorScope("Renderer", defaultAllocator);
	this->renderer = defaultAllocator->MakeNew<Renderer>(rendererAlloc, this->lightManager);

	Allocator* sceneManagerAlloc = allocatorManager->CreateAllocatorScope("SceneManager", defaultAllocator);
	this->sceneManager = defaultAllocator->MakeNew<SceneManager>(sceneManagerAlloc);
}

Engine::~Engine()
{
	Allocator* defaultAllocator = Memory::GetDefaultAllocator();

	defaultAllocator->MakeDelete(this->sceneManager);
	defaultAllocator->MakeDelete(this->renderer);
	defaultAllocator->MakeDelete(this->lightManager);
	defaultAllocator->MakeDelete(this->resourceManager);
	defaultAllocator->MakeDelete(this->materialManager);
	defaultAllocator->MakeDelete(this->meshManager);
	defaultAllocator->MakeDelete(this->entityManager);
	defaultAllocator->MakeDelete(this->time);
	defaultAllocator->MakeDelete(this->debug);
	defaultAllocator->MakeDelete(this->mainWindow);

	defaultAllocator->MakeDelete(this->allocatorManager);

	Memory::DeinitializeMemorySystem();
}

bool Engine::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);
	
	Vec2i windowSize(1600, 900);

	if (this->mainWindow->Initialize(windowSize.x, windowSize.y, "Kokko"))
	{
		const char* const logFilename = "log.txt";
		const char* const debugFontFilename = "res/fonts/gohufont-uni-14.bdf";

		DebugLog* debugLog = this->debug->GetLog();
		debugLog->OpenLogFile(logFilename, false);

		DebugTextRenderer* debugTextRenderer = this->debug->GetTextRenderer();
		bool fontLoaded = debugTextRenderer->LoadBitmapFont(debugFontFilename);
		if (fontLoaded == false)
			debugLog->Log(String("Loading font at ") + debugFontFilename + " failed.");

		debug->SetWindow(mainWindow);

		renderer->Initialize(mainWindow);

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

	unsigned int primarySceneId = this->sceneManager->GetPrimarySceneId();
	Scene* primaryScene = this->sceneManager->GetScene(primarySceneId);

	// Propagate transform updates from Scene to other systems that require it
	ITransformUpdateReceiver* transformUpdateReceivers[] = { this->lightManager, this->renderer };
	unsigned int receiverCount = sizeof(transformUpdateReceivers) / sizeof(transformUpdateReceivers[0]);
	primaryScene->NotifyUpdatedTransforms(receiverCount, transformUpdateReceivers);

	this->renderer->Render(primaryScene);

	this->debug->Render(primaryScene);

	this->mainWindow->UpdateInput();
	this->mainWindow->Swap();

	Debug::CheckOpenGlErrors();
}
