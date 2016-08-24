#include "App.hpp"

#include "Debug.hpp"
#include "DebugLog.hpp"
#include "DebugLogView.hpp"
#include "DebugTextRenderer.hpp"
#include "DebugVectorRenderer.hpp"

#include "World.hpp"
#include "Material.hpp"
#include "MeshLoader.hpp"
#include "Math.hpp"

#include <cstdio>

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

App* App::instance = nullptr;

App::App() :
	world(nullptr),
	debug(nullptr)
{
	App::instance = this;
}

App::~App()
{
	delete debug;
}

bool App::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);

	if (this->mainWindow.Initialize("Kokko"))
	{
		this->world = new World;
		this->world->SetBackgroundColor(Color(0.1f, 0.1f, 0.1f, 1.0f));

		this->debug = new Debug(mainWindow.GetKeyboardInput());

		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);
		
		this->cameraController.SetControlledCamera(&this->mainCamera);

		this->debug->GetLog()->OpenLogFile("log.txt", false);

		DebugTextRenderer* dtr = this->debug->GetTextRenderer();
		dtr->LoadBitmapFont("res/fonts/gohufont-uni-14.bdf");

		Vec2f frameSize = mainWindow.GetFrameBufferSize();
		dtr->SetFrameSize(frameSize);
		dtr->SetScaleFactor(2.0f);
		debug->UpdateLogViewDrawArea();

		// Meshes from files

		ObjectId tableMeshId = this->resourceManager.meshes.Add();
		Mesh& tableMesh = this->resourceManager.meshes.Get(tableMeshId);
		MeshLoader::LoadMesh("res/models/small_table.mesh", tableMesh);

		ObjectId groundMeshId = this->resourceManager.meshes.Add();
		Mesh& groundMesh = this->resourceManager.meshes.Get(groundMeshId);
		MeshLoader::LoadMesh("res/models/ground_plane.mesh", groundMesh);

		ObjectId cupMeshId = this->resourceManager.meshes.Add();
		Mesh& cupMesh = this->resourceManager.meshes.Get(cupMeshId);
		MeshLoader::LoadMesh("res/models/tea_cup.mesh", cupMesh);

		// Materials

		Material* diffuseGray = resourceManager.GetMaterial("res/materials/diffuse_gray.material.json");
		Material* diffuseRed = resourceManager.GetMaterial("res/materials/diffuse_red.material.json");

		// Objects

		SceneObjectId tableSceneObj = scene.AddSceneObject();
		RenderObject& tableRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		tableRenderObj.mesh = tableMeshId;
		tableRenderObj.materialId = diffuseRed->nameHash;
		tableRenderObj.sceneObjectId = tableSceneObj;

		SceneObjectId groundSceneObj = scene.AddSceneObject();
		RenderObject& groundRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		groundRenderObj.mesh = groundMeshId;
		groundRenderObj.materialId = diffuseGray->nameHash;
		groundRenderObj.sceneObjectId = groundSceneObj;

		SceneObjectId cupSceneObj = scene.AddSceneObject();
		RenderObject& cupRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		cupRenderObj.mesh = cupMeshId;
		cupRenderObj.materialId = diffuseGray->nameHash;
		cupRenderObj.sceneObjectId = cupSceneObj;

		Mat4x4f cupTransform = Mat4x4f::Translate(Vec3f(0.0f, 0.439f, 0.0f)) *
		Mat4x4f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), Mathf::DegreesToRadians(135.0f));
		scene.SetLocalTransform(cupSceneObj, cupTransform);

		// Skybox

		Material* skyboxMaterial = resourceManager.GetMaterial("res/materials/skybox.material.json");
		this->world->SetSkyboxMaterialId(skyboxMaterial->nameHash);
		this->world->InitializeSkyboxMesh(&resourceManager);

		this->mainCamera.transform.position = Vec3f(0.0f, 0.3f, 1.5f);
		this->mainCamera.perspectiveFieldOfView = Mathf::DegreesToRadians(45.0f);

		this->mainCamera.SetAspectRatio(frameSize.x, frameSize.y);
		
		return true;
	}
	else
		return false;
}

bool App::HasRequestedQuit()
{
	return this->mainWindow.ShouldClose();
}

void App::Update()
{
	this->time.Update();
	this->cameraController.Update();

	float deltaTime = time.GetDeltaTime();
	float fps = 1.0f / deltaTime;
	float ms = deltaTime * 1000.0f;
	char frameRateText[64];
	sprintf(frameRateText, "%.2f frames per second, %.2f ms per frame", double(fps), double(ms));

	this->debug->GetLog()->Log(StringRef(frameRateText));

	Color green(0.0f, 1.0f, 0.0f, 1.0f);
	Color yellow(1.0f, 1.0f, 0.0f, 1.0f);
	Color white(1.0f, 1.0f, 1.0f, 1.0f);

	DebugVectorRenderer* debugVector = this->debug->GetVectorRenderer();
	debugVector->DrawCube(Mat4x4f(), green);
	debugVector->DrawSphere(Vec3f(), 0.5f, yellow);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, 0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(0.2f, 0.2f, -0.2f), white);
	debugVector->DrawLine(Vec3f(), Vec3f(-0.2f, 0.2f, -0.2f), white);

	this->scene.CalculateWorldTransforms();
	this->renderer.Render(world, &scene);

	this->debug->Render(this->mainCamera);

	this->mainWindow.UpdateInput();

	this->mainWindow.Swap();
}
