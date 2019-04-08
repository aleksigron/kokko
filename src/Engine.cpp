#include "Engine.hpp"

#include <cstdio>

#include "Window.hpp"
#include "Time.hpp"
#include "EntityManager.hpp"
#include "Renderer.hpp"
#include "MeshManager.hpp"
#include "MaterialManager.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"
#include "String.hpp"

#include "Debug.hpp"
#include "DebugLog.hpp"
#include "DebugTextRenderer.hpp"

#include "IncludeGLFW.hpp"

Engine* Engine::instance = nullptr;

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

Engine::Engine()
{
	Engine::instance = this;

	this->mainWindow = new Window;

	this->debug = new Debug;

	this->time = new Time;
	this->entityManager = new EntityManager;
	this->meshManager = new MeshManager;
	this->materialManager = new MaterialManager;
	this->resourceManager = new ResourceManager;
	this->renderer = new Renderer;
	this->sceneManager = new SceneManager;
}

Engine::~Engine()
{
	delete this->sceneManager;
	delete this->renderer;
	delete this->resourceManager;
	delete this->meshManager;
	delete this->entityManager;
	delete this->time;

	delete this->debug;

	delete this->mainWindow;
}

bool Engine::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);
	
	if (this->mainWindow->Initialize("Kokko"))
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

	this->renderer->Render(primaryScene);

	this->debug->Render(primaryScene);

	this->mainWindow->UpdateInput();
	this->mainWindow->Swap();

	Debug::CheckOpenGlErrors();
}
