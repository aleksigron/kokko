#include "Engine.hpp"

#include <cstdio>

#include "Window.hpp"
#include "Time.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

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
	this->renderer = new Renderer;
	this->resourceManager = new ResourceManager;
	this->sceneManager = new SceneManager(this->resourceManager);
}

Engine::~Engine()
{
	delete this->sceneManager;
	delete this->resourceManager;
	delete this->renderer;
	delete this->time;

	delete this->debug;

	delete this->mainWindow;
}

bool Engine::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);
	
	if (this->mainWindow->Initialize("Kokko"))
	{
		DebugLog* debugLog = this->debug->GetLog();
		debugLog->OpenLogFile("log.txt", false);

		DebugTextRenderer* debugTextRenderer = this->debug->GetTextRenderer();
		debugTextRenderer->LoadBitmapFont("res/fonts/gohufont-uni-14.bdf");

		this->debug->SetWindow(this->mainWindow);

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

	this->renderer->PreTransformUpdate(primaryScene);

	primaryScene->CalculateWorldTransforms();
	this->renderer->Render(primaryScene);

	this->debug->Render();

	this->mainWindow->UpdateInput();

	this->mainWindow->Swap();

	this->debug->CheckOpenGlErrors();
}
