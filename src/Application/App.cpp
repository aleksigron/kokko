#include "App.hpp"

#include "Math/Math.hpp"
#include "System/Time.hpp"

#include "Engine/Engine.hpp"
#include "System/Window.hpp"
#include "Rendering/Renderer.hpp"
#include "Entity/EntityManager.hpp"
#include "Scene/SceneManager.hpp"
#include "Rendering/LightManager.hpp"
#include "Scene/Scene.hpp"

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
	LightManager* lightManager = engine->GetLightManager();

	this->cameraController.SetControlledCamera(&this->mainCamera);

	SceneManager* sceneManager = engine->GetSceneManager();
	unsigned int sceneId = sceneManager->LoadSceneFromFile(StringRef("res/scenes/default.scene.json"));

	// Create an empty scene if one couldn't be loaded from file
	if (sceneId == 0)
		sceneId = sceneManager->CreateScene();

	Scene* scene = sceneManager->GetScene(sceneId);
	scene->SetActiveCamera(&this->mainCamera);
	sceneManager->SetPrimarySceneId(sceneId);

	// Create camera entity and components
	Entity mainCameraEntity = entityManager->Create();
	mainCamera.SetEntity(mainCameraEntity);
	SceneObjectId cameraSceneObject = scene->AddSceneObject(mainCameraEntity);
	Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 1.6f, 4.0f));
	scene->SetLocalTransform(cameraSceneObject, cameraTransform);

	{
		// Create directional light entity and components
		Entity lightEntity = entityManager->Create();

		LightId lightId = lightManager->AddLight(lightEntity);
		lightManager->SetLightType(lightId, LightType::Directional);
		lightManager->SetColor(lightId, Vec3f(0.8f, 0.8f, 0.8f));
		lightManager->SetFarDistance(lightId, 100.0f);
		lightManager->SetShadowCasting(lightId, true);

		SceneObjectId lightSceneObject = scene->AddSceneObject(lightEntity);
		Mat4x4f lightTransform = Mat4x4f::LookAt(Vec3f(), Vec3f(-0.4f, -1.0f, -0.6f), Vec3f(0.0f, 1.0f, 0.0f));
		scene->SetLocalTransform(lightSceneObject, lightTransform);
	}

	{
		// Create point light entity and components
		Entity lightEntity = entityManager->Create();

		LightId lightId = lightManager->AddLight(lightEntity);
		lightManager->SetLightType(lightId, LightType::Point);
		lightManager->SetColor(lightId, Vec3f(2.0f, 3.0f, 5.0f));
		lightManager->SetFarDistance(lightId, 10.0f);
		lightManager->SetShadowCasting(lightId, false);

		SceneObjectId lightSceneObject = scene->AddSceneObject(lightEntity);
		Mat4x4f lightTransform = Mat4x4f::Translate(Vec3f(-5.0f, 1.5f, 4.5f));
		scene->SetLocalTransform(lightSceneObject, lightTransform);
	}

	{
		// Create point light entity and components
		Entity lightEntity = entityManager->Create();

		LightId lightId = lightManager->AddLight(lightEntity);
		lightManager->SetLightType(lightId, LightType::Point);
		lightManager->SetColor(lightId, Vec3f(5.0f, 3.0f, 2.0f));
		lightManager->SetFarDistance(lightId, 10.0f);
		lightManager->SetShadowCasting(lightId, false);

		SceneObjectId lightSceneObject = scene->AddSceneObject(lightEntity);
		Mat4x4f lightTransform = Mat4x4f::Translate(Vec3f(5.0f, 1.5f, 4.5f));
		scene->SetLocalTransform(lightSceneObject, lightTransform);
	}

	Window* window = engine->GetMainWindow();
	Vec2f frameSize = window->GetFrameBufferSize();
	mainCamera.parameters.projection = ProjectionType::Perspective;
	mainCamera.parameters.near = 0.1f;
	mainCamera.parameters.far = 100.0f;
	mainCamera.parameters.height = Math::DegreesToRadians(45.0f);
	mainCamera.parameters.SetAspectRatio(frameSize.x, frameSize.y);
}

void App::Update()
{
	if (this->cameraControllerEnable)
		this->cameraController.Update();
}
