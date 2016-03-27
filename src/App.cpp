#include "App.hpp"

#include "ImageData.hpp"
#include "MeshLoader.hpp"

#include "Material.hpp"

#include <cstdio>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

App* App::instance = nullptr;

App::App()
{
	App::instance = this;
}

App::~App()
{
}

bool App::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);

	if (this->mainWindow.Initialize("Kokko"))
	{
		this->input.Initialize(mainWindow.GetWindowHandle());
		this->renderer.Initialize();
		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);
		this->cameraController.SetControlledCamera(&this->mainCamera);

		// Meshes from files

		ObjectId tableMeshId = this->resourceManager.meshes.Add();
		Mesh& tableMesh = this->resourceManager.meshes.Get(tableMeshId);
		MeshLoader::LoadMesh("res/models/small_table.mesh", tableMesh);

		// Material

		Material* diffuseMat = resourceManager.GetMaterial("res/materials/diffuse_gray.material.json");

		// Objects

		SceneObjectId tableSceneObj = scene.AddSceneObject();
		RenderObject& tableRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		tableRenderObj.mesh = tableMeshId;
		tableRenderObj.materialId = diffuseMat->nameHash;
		tableRenderObj.sceneObjectId = tableSceneObj;

		this->mainCamera.transform.position = Vec3f(0.0f, 0.3f, 1.5f);
		this->mainCamera.perspectiveFieldOfView = Mathf::DegreesToRadians(45.0f);

		Vec2i frameSize = this->mainWindow.GetFrameBufferSize();
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
	this->input.Update();
	this->cameraController.Update();

	this->scene.CalculateWorldTransforms();
	this->renderer.Render(this->scene);
	this->mainWindow.Swap();
}
