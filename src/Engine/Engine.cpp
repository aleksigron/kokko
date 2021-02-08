#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"

#include "Entity/EntityManager.hpp"

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

#include "System/Time.hpp"
#include "System/Window.hpp"

Engine* Engine::instance = nullptr;

Engine::Engine()
{
	Engine::instance = this;

	Memory::InitializeMemorySystem();
	Allocator* defaultAllocator = Memory::GetDefaultAllocator();

	this->allocatorManager = defaultAllocator->MakeNew<AllocatorManager>(defaultAllocator);

	Allocator* windowAlloc = allocatorManager->CreateAllocatorScope("Window", defaultAllocator);
	this->mainWindow = defaultAllocator->MakeNew<Window>(windowAlloc);

	this->renderDevice = defaultAllocator->MakeNew<RenderDeviceOpenGL>();

	Allocator* debugAlloc = allocatorManager->CreateAllocatorScope("Debug", defaultAllocator);
	this->debug = defaultAllocator->MakeNew<Debug>(debugAlloc, renderDevice);

	this->time = defaultAllocator->MakeNew<Time>();

	Allocator* entityManagerAlloc = allocatorManager->CreateAllocatorScope("EntityManager", defaultAllocator);
	this->entityManager = defaultAllocator->MakeNew<EntityManager>(entityManagerAlloc);

	Allocator* meshManagerAlloc = allocatorManager->CreateAllocatorScope("MeshManager", defaultAllocator);
	this->meshManager = defaultAllocator->MakeNew<MeshManager>(meshManagerAlloc, renderDevice);

	Allocator* textureManagerAlloc = allocatorManager->CreateAllocatorScope("TextureManager", defaultAllocator);
	this->textureManager = defaultAllocator->MakeNew<TextureManager>(textureManagerAlloc, renderDevice);

	Allocator* shaderManagerAlloc = allocatorManager->CreateAllocatorScope("ShaderManager", defaultAllocator);
	this->shaderManager = defaultAllocator->MakeNew<ShaderManager>(shaderManagerAlloc, renderDevice);

	Allocator* materialManagerAlloc = allocatorManager->CreateAllocatorScope("MaterialManager", defaultAllocator);
	this->materialManager = defaultAllocator->MakeNew<MaterialManager>(
		materialManagerAlloc, renderDevice, shaderManager, textureManager);

	Allocator* lightManagerAlloc = allocatorManager->CreateAllocatorScope("LightManager", defaultAllocator);
	this->lightManager = defaultAllocator->MakeNew<LightManager>(lightManagerAlloc);

	Allocator* rendererAlloc = allocatorManager->CreateAllocatorScope("Renderer", defaultAllocator);
	this->renderer = defaultAllocator->MakeNew<Renderer>(
		rendererAlloc, renderDevice, lightManager, shaderManager, meshManager, materialManager);

	Allocator* sceneManagerAlloc = allocatorManager->CreateAllocatorScope("SceneManager", defaultAllocator);
	this->sceneManager = defaultAllocator->MakeNew<SceneManager>(sceneManagerAlloc);
}

Engine::~Engine()
{
	renderer->Deinitialize();
	debug->Deinitialize();

	Allocator* defaultAllocator = Memory::GetDefaultAllocator();

	defaultAllocator->MakeDelete(this->sceneManager);
	defaultAllocator->MakeDelete(this->renderer);
	defaultAllocator->MakeDelete(this->lightManager);
	defaultAllocator->MakeDelete(this->materialManager);
	defaultAllocator->MakeDelete(this->shaderManager);
	defaultAllocator->MakeDelete(this->textureManager);
	defaultAllocator->MakeDelete(this->meshManager);
	defaultAllocator->MakeDelete(this->entityManager);
	defaultAllocator->MakeDelete(this->time);
	defaultAllocator->MakeDelete(this->debug);
	defaultAllocator->MakeDelete(this->renderDevice);
	defaultAllocator->MakeDelete(this->mainWindow);

	defaultAllocator->MakeDelete(this->allocatorManager);

	Memory::DeinitializeMemorySystem();
}

bool Engine::Initialize()
{
	Vec2i windowSize(1920, 1080);

	if (this->mainWindow->Initialize(windowSize.x, windowSize.y, "Kokko"))
	{
		const char* const logFilename = "log.txt";
		const char* const debugFontFilename = "res/fonts/gohufont-uni-14.bdf";

		DebugLog* debugLog = this->debug->GetLog();
		debugLog->OpenLogFile(logFilename, false);

		DebugTextRenderer* debugTextRenderer = this->debug->GetTextRenderer();
		bool fontLoaded = debugTextRenderer->LoadBitmapFont(textureManager, debugFontFilename);
		if (fontLoaded == false)
		{
			Allocator* defaultAllocator = Memory::GetDefaultAllocator();
			String logText = String(defaultAllocator, "Loading font at ") + debugFontFilename + " failed.";
			debugLog->Log(logText);
		}

		debug->Initialize(mainWindow, meshManager, shaderManager);
		textureManager->Initialize();
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

	static size_t frames = 0;
	if (frames++ % 100 == 0) Debug::CheckOpenGlErrors();
}
