#include "Engine/Engine.hpp"

#include <cstdio>

#include "Core/Core.hpp"
#include "Core/String.hpp"

#include "Debug/Debug.hpp"
#include "Debug/DebugLog.hpp"
#include "Debug/DebugTextRenderer.hpp"
#include "Debug/DebugVectorRenderer.hpp"
#include "Debug/Instrumentation.hpp"

#include "Editor/EditorUI.hpp"

#include "Engine/World.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/EnvironmentManager.hpp"
#include "Graphics/Scene.hpp"

#include "Memory/AllocatorManager.hpp"
#include "Memory/Memory.hpp"
#include "Memory/ProxyAllocator.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/RenderDeviceOpenGL.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/ShaderManager.hpp"
#include "Resources/MaterialManager.hpp"
#include "Resources/TextureManager.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/InputManager.hpp"
#include "System/Time.hpp"
#include "System/Window.hpp"

Engine::Engine()
{
	KOKKO_PROFILE_FUNCTION();

	Memory::InitializeMemorySystem();
	Allocator* alloc = Memory::GetDefaultAllocator();

	allocatorManager = alloc->MakeNew<AllocatorManager>(alloc);

	mainWindow.CreateScope(allocatorManager, "Window", alloc);
	mainWindow.New(mainWindow.allocator);

	editorUI.CreateScope(allocatorManager, "EditorUI", alloc);
	editorUI.New(editorUI.allocator);

	systemAllocator = allocatorManager->CreateAllocatorScope("System", alloc);
	time = systemAllocator->MakeNew<Time>();
	renderDevice = systemAllocator->MakeNew<RenderDeviceOpenGL>();

	debug.CreateScope(allocatorManager, "Debug", alloc);
	debug.New(debug.allocator, allocatorManager, mainWindow.instance, renderDevice);

	debugNameAllocator = allocatorManager->CreateAllocatorScope("EntityDebugNames", alloc);

	meshManager.CreateScope(allocatorManager, "MeshManager", alloc);
	meshManager.New(meshManager.allocator, renderDevice);

	textureManager.CreateScope(allocatorManager, "TextureManager", alloc);
	textureManager.New(textureManager.allocator, renderDevice);

	shaderManager.CreateScope(allocatorManager, "ShaderManager", alloc);
	shaderManager.New(shaderManager.allocator, renderDevice);

	materialManager.CreateScope(allocatorManager, "MaterialManager", alloc);
	materialManager.New(materialManager.allocator, renderDevice, shaderManager.instance, textureManager.instance);

	environmentManager.CreateScope(allocatorManager, "EnvironmentManager", alloc);
	environmentManager.New(environmentManager.allocator, renderDevice,
		shaderManager.instance, meshManager.instance, textureManager.instance);

	ResourceManagers resManagers;
	resManagers.meshManager = meshManager.instance;
	resManagers.shaderManager = shaderManager.instance;
	resManagers.materialManager = materialManager.instance;
	resManagers.textureManager = textureManager.instance;
	resManagers.environmentManager = environmentManager.instance;

	world.CreateScope(allocatorManager, "World", alloc);
	world.New(allocatorManager, world.allocator, debugNameAllocator, renderDevice,
		mainWindow.instance->GetInputManager(), resManagers);
}

Engine::~Engine()
{
	world.instance->Deinitialize();
	debug.instance->Deinitialize();
	editorUI.instance->Deinitialize();

	world.Delete();
	environmentManager.Delete();
	materialManager.Delete();
	shaderManager.Delete();
	textureManager.Delete();
	meshManager.Delete();
	debug.Delete();
	systemAllocator->MakeDelete(this->time);
	systemAllocator->MakeDelete(this->renderDevice);
	editorUI.Delete();
	mainWindow.Delete();

	Allocator* defaultAllocator = Memory::GetDefaultAllocator();
	defaultAllocator->MakeDelete(this->allocatorManager);

	Memory::DeinitializeMemorySystem();
}

bool Engine::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	Vec2i windowSize(1920, 1080);

	if (mainWindow.instance->Initialize(windowSize.x, windowSize.y, "Kokko"))
	{
		ResourceManagers resManagers;
		resManagers.meshManager = meshManager.instance;
		resManagers.shaderManager = shaderManager.instance;
		resManagers.materialManager = materialManager.instance;
		resManagers.textureManager = textureManager.instance;
		resManagers.environmentManager = environmentManager.instance;

		editorUI.instance->Initialize(mainWindow.instance, resManagers);

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

		debug.instance->Initialize(mainWindow.instance, meshManager.instance, shaderManager.instance);

		textureManager.instance->Initialize();
		environmentManager.instance->Initialize();

		world.instance->Initialize(mainWindow.instance);

		return true;
	}
	else
	{
		return false;
	}
}

void Engine::FrameStart()
{
	if (debug.instance->ShouldBeginProfileSession())
		Instrumentation::Get().BeginSession("runtime_trace.json");

	if (debug.instance->ShouldEndProfileSession())
		Instrumentation::Get().EndSession();

	editorUI.instance->StartFrame();
}

void Engine::Update()
{
	KOKKO_PROFILE_FUNCTION();

	// FRAME UPDATE

	this->time->Update();

	world.instance->Update();

	// Because editor can change the state of the world and systems,
	// let's run those updates at the same part of the frame as other updates
	editorUI.instance->Update(world.instance);

	// FRAME RENDER

	ViewRectangle viewport = editorUI.instance->GetWorldViewport();
	Mat4x4fBijection editorCameraTransform = editorUI.instance->GetEditorCameraTransform();
	ProjectionParameters editorCameraProjection = editorUI.instance->GetEditorCameraProjection();
	editorCameraProjection.SetAspectRatio(viewport.size.x, viewport.size.y);
	Optional<CameraParameters> editorCamera = Optional(CameraParameters{ editorCameraTransform, editorCameraProjection });

	world.instance->Render(editorCamera, viewport);
	world.instance->DebugRender(debug.instance->GetVectorRenderer());

	debug.instance->Render(world.instance, viewport, editorCamera);

	// FRAME END

	editorUI.instance->EndFrame();

	mainWindow.instance->UpdateInput();
	mainWindow.instance->Swap();
}

void Engine::SetAppPointer(void* app)
{
	world.instance->GetScriptSystem()->SetAppPointer(app);
}
