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
		Vec2f frameSize = window->GetFrameBufferSize().As<float>();
		mainCamera.parameters.projection = ProjectionType::Perspective;
		mainCamera.parameters.near = 0.1f;
		mainCamera.parameters.far = 500.0f;
		mainCamera.parameters.height = Math::DegreesToRadians(60.0f);
		mainCamera.parameters.SetAspectRatio(frameSize.x, frameSize.y);
	}

	// Load mesh and material for our additional objects

	StringRef meshPath("res/models/simple_sphere.mesh");
	MeshId meshId = meshManager->GetIdByPath(meshPath);

	StringRef matPath("res/materials/deferred_geometry/standard_gray_m00_r00.material.json");
	MaterialId origMatId = materialManager->GetIdByPath(matPath);
	const MaterialData& origMatData = materialManager->GetMaterialData(origMatId);

	if (meshId.IsValid() && origMatId.IsNull() == false)
	{
		const char* metalnessStr = "metalness";
		const char* roughnessStr = "roughness";
		uint32_t metalnessHash = Hash::FNV1a_32(metalnessStr, std::strlen(metalnessStr));
		uint32_t roughnessHash = Hash::FNV1a_32(roughnessStr, std::strlen(roughnessStr));

		const BufferUniform* metalUniform = origMatData.uniforms.FindBufferUniformByNameHash(metalnessHash);
		const BufferUniform* roughUniform = origMatData.uniforms.FindBufferUniformByNameHash(roughnessHash);

		static const unsigned int roughnessCount = 16;
		static const unsigned int metalnessCount = 4;
		MaterialId materialIds[roughnessCount * metalnessCount];

		for (unsigned int m = 0; m < metalnessCount; ++m)
		{
			for (unsigned int r = 0; r < roughnessCount; ++r)
			{
				MaterialId matId = materialManager->CreateCopy(origMatId);
				const MaterialData& materialData = materialManager->GetMaterialData(matId);

				float metalness = m / (metalnessCount - 1.0f);
				float roughness = 0.1f + r / (roughnessCount - 1.0f) * 0.9f;

				metalUniform->SetValueFloat(materialData.uniformData, metalness);
				roughUniform->SetValueFloat(materialData.uniformData, roughness);

				materialManager->UpdateUniformsToGPU(matId);

				materialIds[m * roughnessCount + r] = matId;
			}
		}

		RenderOrderData renderOrderData;
		renderOrderData.material = origMatId;
		renderOrderData.transparency = materialManager->GetMaterialData(origMatId).transparency;

		const float dist = 1.2f;
		const float innerRadius = 5.0f;
		float a = Math::Const::Pi;

		// Add a bunch of objects to the world
		for (unsigned int i = 0, count = 1000; i < count; ++i)
		{
			float r = innerRadius + a / Math::Const::Tau * dist;
			a += dist / r;

			unsigned int metal = static_cast<unsigned int>(a / Math::Const::Tau * metalnessCount) % metalnessCount;
			unsigned int rough = static_cast<unsigned int>((r - innerRadius) / dist) % roughnessCount;

			Entity entity = entityManager->Create();

			SceneObjectId sceneObject = scene->AddSceneObject(entity);
			Mat4x4f transform = Mat4x4f::Translate(Vec3f(std::sin(a) * r, 0.5f, std::cos(a) * r));
			scene->SetLocalTransform(sceneObject, transform);

			RenderObjectId renderObj = renderer->AddRenderObject(entity);
			renderOrderData.material = materialIds[metal * roughnessCount + rough];
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
