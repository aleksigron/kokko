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

	Renderer* renderer = engine->GetRenderer();
	renderer->SetActiveCamera(&this->mainCamera);

	DebugVectorRenderer* debugVectorRenderer = engine->GetDebug()->GetVectorRenderer();
	debugVectorRenderer->SetActiveCamera(&this->mainCamera);

	this->cameraController.SetControlledCamera(&this->mainCamera);

	// Meshes from files

	ResourceManager* rm = engine->GetResourceManager();

	ObjectId tableMeshId = rm->meshes.Add();
	Mesh& tableMesh = rm->meshes.Get(tableMeshId);
	MeshLoader::LoadMesh("res/models/small_table.mesh", tableMesh);

	ObjectId groundMeshId = rm->meshes.Add();
	Mesh& groundMesh = rm->meshes.Get(groundMeshId);
	MeshLoader::LoadMesh("res/models/ground_plane.mesh", groundMesh);

	ObjectId cupMeshId = rm->meshes.Add();
	Mesh& cupMesh = rm->meshes.Get(cupMeshId);
	MeshLoader::LoadMesh("res/models/tea_cup.mesh", cupMesh);

	// Materials

	unsigned int diffuseGray = rm->CreateMaterialFromFile("res/materials/diffuse_gray.material.json");
	unsigned int diffuseRed = rm->CreateMaterialFromFile("res/materials/diffuse_red.material.json");

	// Objects

	Scene* scene = engine->GetScene();

	scene->backgroundColor = Color(0.1f, 0.1f, 0.1f, 1.0f);

	unsigned int tableSceneObj = scene->AddSceneObject();
	RenderObject& tableRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	tableRenderObj.meshId = tableMeshId;
	tableRenderObj.materialId = diffuseRed;
	tableRenderObj.sceneObjectId = tableSceneObj;
	tableRenderObj.layer = 0;

	unsigned int groundSceneObj = scene->AddSceneObject();
	RenderObject& groundRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	groundRenderObj.meshId = groundMeshId;
	groundRenderObj.materialId = diffuseGray;
	groundRenderObj.sceneObjectId = groundSceneObj;
	groundRenderObj.layer = 0;

	unsigned int cupSceneObj = scene->AddSceneObject();
	RenderObject& cupRenderObj = renderer->GetRenderObject(renderer->AddRenderObject());
	cupRenderObj.meshId = cupMeshId;
	cupRenderObj.materialId = diffuseGray;
	cupRenderObj.sceneObjectId = cupSceneObj;
	cupRenderObj.layer = 0;

	Mat4x4f cupTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.439f, 0.0f)) *
	Mat4x4f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), Math::DegreesToRadians(135.0f));
	scene->SetLocalTransform(cupSceneObj, cupTransform);

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
