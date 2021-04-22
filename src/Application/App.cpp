#include "App.hpp"

#include "Application/CameraController.hpp"

#include "Core/Core.hpp"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Math/Math.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/Renderer.hpp"
#include "Rendering/LightManager.hpp"

#include "Resources/MeshManager.hpp"
#include "Resources/MaterialManager.hpp"

#include "Scene/Scene.hpp"
#include "Scene/SceneManager.hpp"

#include "Scripting/ScriptSystem.hpp"

#include "System/Time.hpp"
#include "System/Window.hpp"

App::App(Engine* engine, Allocator* allocator) :
	engine(engine),
	allocator(allocator),
	settings(allocator),
	mainCameraEntity(Entity::Null),
	cameraControllerEnable(true)
{
}

App::~App()
{
}

void App::Initialize()
{
	KOKKO_PROFILE_FUNCTION();

	settings.SetFilename(StringRef("global.settings"));
	settings.LoadFromFile();

	EntityManager* entityManager = engine->GetEntityManager();
	MaterialManager* materialManager = engine->GetMaterialManager();
	MeshManager* meshManager = engine->GetMeshManager();
	Renderer* renderer = engine->GetRenderer();

	SceneManager* sceneManager = engine->GetSceneManager();
	unsigned int sceneId = sceneManager->LoadSceneFromFile(StringRef("res/scenes/test.scene.json"));

	// Create an empty scene if one couldn't be loaded from file
	if (sceneId == 0)
		sceneId = sceneManager->CreateScene();

	Scene* scene = sceneManager->GetScene(sceneId);
	sceneManager->SetPrimarySceneId(sceneId);

	// Create camera entity and components

	mainCameraEntity = entityManager->Create();

	CameraSystem* cameraSystem = engine->GetCameraSystem();
	CameraId mainCameraId = cameraSystem->AddCameraComponent(mainCameraEntity);

	Vec2f frameSize = engine->GetMainWindow()->GetFrameBufferSize().As<float>();
	ProjectionParameters projection;
	projection.SetPerspective(Math::DegreesToRadians(60.0f));
	projection.perspectiveNear = 0.1f;
	projection.perspectiveFar = 10000.0f;
	projection.SetAspectRatio(frameSize.x, frameSize.y);

	cameraSystem->SetProjectionParameters(mainCameraId, projection);
		
	SceneObjectId cameraSceneObject = scene->AddSceneObject(mainCameraEntity);
	Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 1.6f, 4.0f));
	scene->SetLocalTransform(cameraSceneObject, cameraTransform);

	scene->SetActiveCameraEntity(mainCameraEntity);

	ScriptSystem* scriptSystem = engine->GetScriptSystem();
	CameraController* controller = scriptSystem->AddScript<CameraController>(mainCameraEntity);
	controller->SetControlledCamera(mainCameraEntity);

	// Load mesh and material for our additional objects

	StringRef meshPath("res/models/simple_sphere.mesh");
	MeshId meshId = meshManager->GetIdByPath(meshPath);

	StringRef matPath("res/materials/deferred_geometry/standard_red_m00_r05.material.json");
	MaterialId origMatId = materialManager->GetIdByPath(matPath);
	const MaterialData& origMatData = materialManager->GetMaterialData(origMatId);

	if (meshId != MeshId::Null && origMatId != MaterialId::Null)
	{
		const char* metalnessStr = "metalness";
		const char* roughnessStr = "roughness";
		uint32_t metalnessHash = Hash::FNV1a_32(metalnessStr, std::strlen(metalnessStr));
		uint32_t roughnessHash = Hash::FNV1a_32(roughnessStr, std::strlen(roughnessStr));

		const BufferUniform* metalUniform = origMatData.uniforms.FindBufferUniformByNameHash(metalnessHash);
		const BufferUniform* roughUniform = origMatData.uniforms.FindBufferUniformByNameHash(roughnessHash);

		static const unsigned int roughnessCount = 9;
		static const unsigned int metalnessCount = 9;

		RenderOrderData renderOrderData;
		renderOrderData.material = origMatId;
		renderOrderData.transparency = materialManager->GetMaterialData(origMatId).transparency;

		for (unsigned int m = 0; m < metalnessCount; ++m)
		{
			for (unsigned int r = 0; r < roughnessCount; ++r)
			{
				MaterialId matId = materialManager->CreateCopy(origMatId);
				const MaterialData& materialData = materialManager->GetMaterialData(matId);

				float metalness = m / (metalnessCount - 1.0f);
				float roughness = 0.05f + r / (roughnessCount - 1.0f) * 0.95f;

				metalUniform->SetValueFloat(materialData.uniformData, metalness);
				roughUniform->SetValueFloat(materialData.uniformData, roughness);

				materialManager->UpdateUniformsToGPU(matId);

				Entity entity = entityManager->Create();

				SceneObjectId sceneObject = scene->AddSceneObject(entity);
				Vec3f position(m * 1.4f - metalnessCount * 0.7f + 0.7f, 0.6f, r * 1.4f - roughnessCount * 0.7f + 0.7f);
				Mat4x4f transform = Mat4x4f::Translate(position);
				scene->SetEditTransform(sceneObject, SceneEditTransform(position));

				RenderObjectId renderObj = renderer->AddRenderObject(entity);
				renderOrderData.material = matId;
				renderer->SetOrderData(renderObj, renderOrderData);
				renderer->SetMeshId(renderObj, meshId);
			}
		}
	}
}

void App::Update()
{
}
