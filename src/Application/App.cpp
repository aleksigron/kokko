#include "App.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Math/Math.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

#include "System/Window.hpp"

App* App::instance = nullptr;

App::App(Allocator* allocator) :
	allocator(allocator),
	settings(allocator),
	cameraControllerEnable(true)
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
	EntityManager* entityManager = engine->GetEntityManager();

	this->cameraController.SetControlledCamera(&this->mainCamera);

	SceneManager* sceneManager = engine->GetSceneManager();
	unsigned int sceneId = sceneManager->LoadSceneFromFile(StringRef("res/scenes/test.scene.json"));

	// Create an empty scene if one couldn't be loaded from file
	if (sceneId == 0)
		sceneId = sceneManager->CreateScene();

	Scene* scene = sceneManager->GetScene(sceneId);
	scene->SetActiveCamera(&this->mainCamera);
	sceneManager->SetPrimarySceneId(sceneId);

	{
		// Create camera entity and components
		Entity mainCameraEntity = entityManager->Create();
		mainCamera.SetEntity(mainCameraEntity);
		SceneObjectId cameraSceneObject = scene->AddSceneObject(mainCameraEntity);
		Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 1.6f, 4.0f));
		scene->SetLocalTransform(cameraSceneObject, cameraTransform);
	}

	{
		Window* window = engine->GetMainWindow();
		Vec2f frameSize = window->GetFrameBufferSize();
		mainCamera.parameters.projection = ProjectionType::Perspective;
		mainCamera.parameters.near = 0.1f;
		mainCamera.parameters.far = 100.0f;
		mainCamera.parameters.height = Math::DegreesToRadians(60.0f);
		mainCamera.parameters.SetAspectRatio(frameSize.x, frameSize.y);
	}
}

void App::Update()
{
	if (this->cameraControllerEnable)
		this->cameraController.Update();
}
