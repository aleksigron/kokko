#include "App.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Math/Math.hpp"

#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

#include "System/Time.hpp"
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
	LightManager* lightManager = engine->GetLightManager();
	MaterialManager* materialManager = engine->GetMaterialManager();
	MeshManager* meshManager = engine->GetMeshManager();
	Renderer* renderer = engine->GetRenderer();

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
		mainCamera.parameters.height = Math::DegreesToRadians(45.0f);
		mainCamera.parameters.SetAspectRatio(frameSize.x, frameSize.y);
	}

	// Load mesh and material for our additional objects

	StringRef meshPath("res/models/sphere.mesh");
	MeshId meshId = meshManager->GetIdByPath(meshPath);

	StringRef matPath("res/materials/deferred_geometry/wood_floor.material.json");
	MaterialId matId = materialManager->GetIdByPath(matPath);

	if (meshId.IsValid() && !matId.IsNull())
	{
		RenderOrderData renderOrderData;
		renderOrderData.material = matId;
		renderOrderData.transparency = materialManager->GetMaterialData(matId).transparency;

		const float dist = 1.0f;
		float a = Math::Const::Pi;

		// Add a bunch of objects to the world
		for (unsigned int i = 0, count = 1000; i < count; ++i)
		{
			float r = 6.8f + a / Math::Const::Tau * dist;
			a += dist / r;

			Entity entity = entityManager->Create();

			SceneObjectId sceneObject = scene->AddSceneObject(entity);
			Mat4x4f transform = Mat4x4f::Translate(Vec3f(std::sin(a) * r, 0.4f, std::cos(a) * r));
			scene->SetLocalTransform(sceneObject, transform);

			RenderObjectId renderObj = renderer->AddRenderObject(entity);
			renderer->SetOrderData(renderObj, renderOrderData);
			renderer->SetMeshId(renderObj, meshId);
		}
	}
}

void App::Update()
{
	if (this->cameraControllerEnable)
		this->cameraController.Update();
}
