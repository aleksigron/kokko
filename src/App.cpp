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

		this->debugTextRenderer.LoadBitmapFont("res/fonts/gohufont-uni-14.bdf");

		Vec2i frameSize = mainWindow.GetFrameBufferSize();
		debugTextRenderer.SetFrameSize(Vec2f(frameSize.x, frameSize.y));
		debugTextRenderer.SetScaleFactor(2.0f);

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
		Material* concreteMat = resourceManager.GetMaterial("res/materials/concrete.material.json");

		// Objects

		SceneObjectId tableSceneObj = scene.AddSceneObject();
		RenderObject& tableRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		tableRenderObj.mesh = tableMeshId;
		tableRenderObj.materialId = concreteMat->nameHash;
		tableRenderObj.sceneObjectId = tableSceneObj;

		SceneObjectId groundSceneObj = scene.AddSceneObject();
		RenderObject& groundRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		groundRenderObj.mesh = groundMeshId;
		groundRenderObj.materialId = diffuseGray->nameHash;
		groundRenderObj.sceneObjectId = groundSceneObj;

		SceneObjectId cupSceneObj = scene.AddSceneObject();
		RenderObject& cupRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		cupRenderObj.mesh = cupMeshId;
		cupRenderObj.materialId = diffuseRed->nameHash;
		cupRenderObj.sceneObjectId = cupSceneObj;

		Mat4x4f cupTransform = Matrix::Translate(Vec3f(0.0f, 0.439f, 0.0f)) *
		Mat4x4f::RotateAroundAxis(Vec3f(0.0f, 1.0f, 0.0f), Mathf::DegreesToRadians(135.0f));
		scene.SetLocalTransform(cupSceneObj, cupTransform);

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
	this->input.Update();
	this->cameraController.Update();

	this->debugTextRenderer.AddText(StringRef("Morbi condimentum, mauris vitae accumsan ullamcorper, risus nunc vulputate lectus, condimentum hendrerit arcu nulla id enim. Praesent sagittis, dui et semper pharetra, ligula mauris convallis nulla, eu blandit orci nulla non ipsum. Praesent porttitor ipsum sit amet nunc mollis, vitae rhoncus mauris ornare."), Vec2f(0.0f, 0.0f));

	this->scene.CalculateWorldTransforms();
	this->renderer.Render(this->scene);

	this->debugTextRenderer.Render();

	this->mainWindow.Swap();
}
