#include "App.hpp"

#include "Engine.hpp"

#include "Window.hpp"

#include "Debug.hpp"
#include "DebugVectorRenderer.hpp"

#include "Math.hpp"

#include "Time.hpp"

#include "Renderer.hpp"

#include "Material.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

#include <cstdio>

App* App::instance = nullptr;

App::App()
{
	App::instance = this;
}

App::~App()
{
}

void App::Initialize()
{
	settings.SetFilename(StringRef("global.settings"));
	settings.LoadFromFile();

	Engine* engine = Engine::GetInstance();

	DebugVectorRenderer* debugVectorRenderer = engine->GetDebug()->GetVectorRenderer();
	debugVectorRenderer->SetActiveCamera(&this->mainCamera);

	this->cameraController.SetControlledCamera(&this->mainCamera);

	SceneManager* sceneManager = engine->GetSceneManager();
	unsigned int sceneId = sceneManager->LoadSceneFromFile("res/scenes/default.scene.json");

	if (sceneId == 0) // Create an empty scene if one couldn't be loaded from file
		sceneId = sceneManager->CreateScene();

	Scene* scene = sceneManager->GetScene(sceneId);
	scene->SetActiveCamera(&this->mainCamera);
	sceneManager->SetPrimarySceneId(sceneId);

	this->mainCamera.InitializeSceneObject(sceneId);
	Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.3f, 1.5f));
	scene->SetLocalTransform(this->mainCamera.GetSceneObjectId(), cameraTransform);

	Window* window = engine->GetMainWindow();
	Vec2f frameSize = window->GetFrameBufferSize();
	this->mainCamera.perspectiveFieldOfView = Math::DegreesToRadians(45.0f);
	this->mainCamera.SetAspectRatio(frameSize.x, frameSize.y);
}

void App::Update()
{
	Engine* engine = Engine::GetInstance();
	Debug* debug = engine->GetDebug();

	this->cameraController.Update();

	Color green(0.0f, 1.0f, 0.0f, 1.0f);
	Color yellow(1.0f, 1.0f, 0.0f, 1.0f);
	Color white(1.0f, 1.0f, 1.0f, 1.0f);

	DebugVectorRenderer* debugVector = debug->GetVectorRenderer();
	debugVector->DrawWireCube(Mat4x4f(), green);
	debugVector->DrawWireSphere(Vec3f(), 0.5f, yellow);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, -0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, -0.2f), white);
}
