#include "App.hpp"

#include "Engine.hpp"

#include "Window.hpp"

#include "Debug.hpp"
#include "DebugVectorRenderer.hpp"

#include "Math.hpp"

#include "Time.hpp"

#include "Renderer.hpp"

#include "Scene.hpp"
#include "Material.hpp"
#include "ResourceManager.hpp"

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
	Engine* engine = Engine::GetInstance();

	Renderer* renderer = engine->GetRenderer();
	renderer->SetActiveCamera(&this->mainCamera);

	DebugVectorRenderer* debugVectorRenderer = engine->GetDebug()->GetVectorRenderer();
	debugVectorRenderer->SetActiveCamera(&this->mainCamera);

	this->cameraController.SetControlledCamera(&this->mainCamera);

	// Meshes from files

	ResourceManager* rm = engine->GetResourceManager();

	unsigned int tableMeshId = rm->CreateMeshFromFile("res/models/small_table.mesh");
	unsigned int groundMeshId = rm->CreateMeshFromFile("res/models/ground_plane.mesh");
	unsigned int cupMeshId = rm->CreateMeshFromFile("res/models/tea_cup.mesh");
	unsigned int sphereMeshId = rm->CreateMeshFromFile("res/models/sphere.mesh");

	// Materials

	unsigned int diffuseGray = rm->CreateMaterialFromFile("res/materials/diffuse_gray.material.json");
	unsigned int diffuseRed = rm->CreateMaterialFromFile("res/materials/diffuse_red.material.json");
	unsigned int blend = rm->CreateMaterialFromFile("res/materials/blend.material.json");

	// Objects

	Scene* scene = engine->GetScene();

	scene->backgroundColor = Color(0.1f, 0.1f, 0.1f, 1.0f);

	unsigned int tableSceneObj = scene->AddSceneObject();
	RenderObject& tableRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	tableRenderObj.meshId = tableMeshId;
	tableRenderObj.materialId = diffuseRed;
	tableRenderObj.sceneObjectId = tableSceneObj;
	tableRenderObj.layer = SceneLayer::World;

	unsigned int groundSceneObj = scene->AddSceneObject();
	RenderObject& groundRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	groundRenderObj.meshId = groundMeshId;
	groundRenderObj.materialId = diffuseGray;
	groundRenderObj.sceneObjectId = groundSceneObj;
	groundRenderObj.layer = SceneLayer::World;

	unsigned int cupSceneObj = scene->AddSceneObject();
	RenderObject& cupRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	cupRenderObj.meshId = cupMeshId;
	cupRenderObj.materialId = diffuseGray;
	cupRenderObj.sceneObjectId = cupSceneObj;
	cupRenderObj.layer = SceneLayer::World;

	unsigned int sphereSceneObj = scene->AddSceneObject();
	RenderObject& sphereRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	sphereRenderObj.meshId = sphereMeshId;
	sphereRenderObj.materialId = blend;
	sphereRenderObj.sceneObjectId = sphereSceneObj;
	sphereRenderObj.layer = SceneLayer::World;

	Mat4x4f cupTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.439f, 0.0f)) *
	Mat4x4f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), Math::DegreesToRadians(135.0f));
	scene->SetLocalTransform(cupSceneObj, cupTransform);

	scene->SetLocalTransform(sphereSceneObj, Mat4x4f::Translate(Vec3f(0.0f, 1.0f, 0.0f)));

	// Skybox

	unsigned int skyboxMaterial = rm->CreateMaterialFromFile("res/materials/skybox.material.json");
	scene->skybox.Initialize(scene, skyboxMaterial);

	// Camera

	this->mainCamera.perspectiveFieldOfView = Math::DegreesToRadians(45.0f);
	
	this->mainCamera.InitializeSceneObject();
	Mat4x4f cameraTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.3f, 1.5f));
	scene->SetLocalTransform(this->mainCamera.GetSceneObjectId(), cameraTransform);

	Window* window = engine->GetMainWindow();
	Vec2f frameSize = window->GetFrameBufferSize();
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
	debugVector->DrawCube(Mat4x4f(), green);
	debugVector->DrawSphere(Vec3f(), 0.5f, yellow);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, -0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, -0.2f), white);
}
