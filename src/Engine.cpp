#include "Engine.hpp"

#include <cstdio>

#include "Window.hpp"
#include "Time.hpp"
#include "Scene.hpp"
#include "Renderer.hpp"
#include "ResourceManager.hpp"

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
	this->scene = new Scene;
	this->renderer = new Renderer;
	this->resourceManager = new ResourceManager;
}

Engine::~Engine()
{
	delete this->resourceManager;
	delete this->renderer;
	delete this->scene;
	delete this->time;

	delete this->debug;

	delete this->mainWindow;
}

bool Engine::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);
	
	if (this->mainWindow->Initialize("Kokko"))
	{
		this->debug->SetKeyboardInput(this->mainWindow->GetKeyboardInput());

		DebugLog* debugLog = this->debug->GetLog();
		debugLog->OpenLogFile("log.txt", false);

		DebugTextRenderer* debugTextRenderer = this->debug->GetTextRenderer();
		debugTextRenderer->LoadBitmapFont("res/fonts/gohufont-uni-14.bdf");

		this->renderer->AttachTarget(this->mainWindow);

		Vec2f frameSize = this->mainWindow->GetFrameBufferSize();
		debugTextRenderer->SetFrameSize(frameSize);
		debugTextRenderer->SetScaleFactor(2.0f);
		debug->UpdateLogViewDrawArea();

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

	this->renderer->PreTransformUpdate();
	this->scene->CalculateWorldTransforms();
	this->renderer->Render(scene);

	this->debug->Render();

	this->mainWindow->UpdateInput();

	this->mainWindow->Swap();

}
