#include "App.hpp"

#include "Engine.hpp"

#include "Window.hpp"

#include "Debug.hpp"
#include "DebugLog.hpp"
#include "DebugLogView.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugVectorRenderer.hpp"

#include "Math.hpp"

#include "Time.hpp"

#include "Renderer.hpp"

#include "Scene.hpp"
#include "World.hpp"
#include "Material.hpp"
#include "MeshLoader.hpp"
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

	World* world = engine->GetWorld();
	world->backgroundColor = Color(0.1f, 0.1f, 0.1f, 1.0f);

	Renderer* renderer = engine->GetRenderer();
	renderer->SetActiveCamera(&this->mainCamera);

	DebugVectorRenderer* debugVectorRenderer = engine->GetDebug()->GetVectorRenderer();
	debugVectorRenderer->SetActiveCamera(&this->mainCamera);

	this->cameraController.SetControlledCamera(&this->mainCamera);

	// Meshes from files

	ResourceManager* resourceManager = engine->GetResourceManager();

	ObjectId tableMeshId = resourceManager->meshes.Add();
	Mesh& tableMesh = resourceManager->meshes.Get(tableMeshId);
	MeshLoader::LoadMesh("res/models/small_table.mesh", tableMesh);

	ObjectId groundMeshId = resourceManager->meshes.Add();
	Mesh& groundMesh = resourceManager->meshes.Get(groundMeshId);
	MeshLoader::LoadMesh("res/models/ground_plane.mesh", groundMesh);

	ObjectId cupMeshId = resourceManager->meshes.Add();
	Mesh& cupMesh = resourceManager->meshes.Get(cupMeshId);
	MeshLoader::LoadMesh("res/models/tea_cup.mesh", cupMesh);

	// Materials

	Material* diffuseGray = resourceManager->GetMaterial("res/materials/diffuse_gray.material.json");
	Material* diffuseRed = resourceManager->GetMaterial("res/materials/diffuse_red.material.json");

	// Objects

	Scene* scene = engine->GetScene();

	SceneObjectId tableSceneObj = scene->AddSceneObject();
	RenderObject& tableRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	tableRenderObj.mesh = tableMeshId;
	tableRenderObj.materialId = diffuseRed->nameHash;
	tableRenderObj.sceneObjectId = tableSceneObj;

	SceneObjectId groundSceneObj = scene->AddSceneObject();
	RenderObject& groundRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	groundRenderObj.mesh = groundMeshId;
	groundRenderObj.materialId = diffuseGray->nameHash;
	groundRenderObj.sceneObjectId = groundSceneObj;

	SceneObjectId cupSceneObj = scene->AddSceneObject();
	RenderObject& cupRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	cupRenderObj.mesh = cupMeshId;
	cupRenderObj.materialId = diffuseGray->nameHash;
	cupRenderObj.sceneObjectId = cupSceneObj;

	Mat4x4f cupTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.439f, 0.0f)) *
	Mat4x4f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), Mathf::DegreesToRadians(135.0f));
	scene->SetLocalTransform(cupSceneObj, cupTransform);

	// Skybox

	Material* skyboxMaterial = resourceManager->GetMaterial("res/materials/skybox.material.json");
	world->skybox.Initialize(scene, skyboxMaterial->nameHash);

	this->mainCamera.transform.position = Vec3f(0.0f, 0.3f, 1.5f);
	this->mainCamera.perspectiveFieldOfView = Mathf::DegreesToRadians(45.0f);

	Window* window = engine->GetMainWindow();
	Vec2f frameSize = window->GetFrameBufferSize();
	this->mainCamera.SetAspectRatio(frameSize.x, frameSize.y);
}

void App::Update()
{
	Engine* engine = Engine::GetInstance();

	this->cameraController.Update();

	Time* time = engine->GetTime();
	float deltaTime = time->GetDeltaTime();
	float fps = 1.0f / deltaTime;
	float ms = deltaTime * 1000.0f;
	char frameRateText[64];
	sprintf(frameRateText, "%.1f frames per second, %.1f ms per frame", double(fps), double(ms));

	Debug* debug = engine->GetDebug();
	debug->GetLog()->Log(StringRef(frameRateText));

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
