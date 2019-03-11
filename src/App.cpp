#include "App.hpp"

#include "Engine.hpp"

#include "Window.hpp"

#include "Debug.hpp"
#include "DebugVectorRenderer.hpp"

#include "Math.hpp"

#include "Time.hpp"

#include "Renderer.hpp"
#include "EntityManager.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "Scene.hpp"

#include <cstdio>

App* App::instance = nullptr;

App::App() : cameraControllerEnable(true)
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

	this->cameraController.SetControlledCamera(&this->mainCamera);

	SceneManager* sceneManager = engine->GetSceneManager();
	unsigned int sceneId = sceneManager->LoadSceneFromFile("res/scenes/default.scene.json");

	// Create an empty scene if one couldn't be loaded from file
	if (sceneId == 0)
		sceneId = sceneManager->CreateScene();

	Scene* scene = sceneManager->GetScene(sceneId);
	scene->SetActiveCamera(&this->mainCamera);
	sceneManager->SetPrimarySceneId(sceneId);

	Entity mainCameraEntity = engine->GetEntityManager()->Create();
	mainCamera.SetEntity(mainCameraEntity);
	SceneObjectId cameraSceneObject = scene->AddSceneObject(mainCameraEntity);
	Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.8f, 2.5f));
	scene->SetLocalTransform(cameraSceneObject, cameraTransform);

	Window* window = engine->GetMainWindow();
	Vec2f frameSize = window->GetFrameBufferSize();
	this->mainCamera.perspectiveFieldOfView = Math::DegreesToRadians(45.0f);
	this->mainCamera.SetAspectRatio(frameSize.x, frameSize.y);
}

void App::Update()
{
	if (this->cameraControllerEnable)
		this->cameraController.Update();
}
