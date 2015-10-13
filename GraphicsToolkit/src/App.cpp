#include "App.h"

#include "ImageData.h"
#include "MeshLoader.h"

#include <cstdio>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

static void OnGlfwError(int errorCode, const char* description)
{
	printf("Error: %d, \"%s\"", errorCode, description);
}

App* App::instance = nullptr;

App::App() : stackAllocator(1 << 14)
{
	App::instance = this;
}

App::~App()
{
}

bool App::Initialize()
{
	glfwSetErrorCallback(OnGlfwError);

	if (this->mainWindow.Initialize())
	{
		this->input.Initialize(mainWindow.GetWindowHandle());
		this->renderer.Initialize();
		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);
		this->cameraController.SetControlledCamera(&this->mainCamera);

		// Meshes from files

		ObjectId groundMeshId = this->resourceManager.meshes.Add();
		Mesh& groundMesh = this->resourceManager.meshes.Get(groundMeshId);
		MeshLoader::LoadMesh("res/models/ground_plane.mesh", groundMesh);

		ObjectId sphereMeshId = this->resourceManager.meshes.Add();
		Mesh& sphereMesh = this->resourceManager.meshes.Get(sphereMeshId);
		MeshLoader::LoadMesh("res/models/sphere.mesh", sphereMesh);

		ObjectId icoMeshId = this->resourceManager.meshes.Add();
		Mesh& icoMesh = this->resourceManager.meshes.Get(icoMeshId);
		MeshLoader::LoadMesh("res/models/icosahedron.mesh", icoMesh);

		ObjectId cubeMeshId = this->resourceManager.meshes.Add();
		Mesh& cubeMesh = this->resourceManager.meshes.Get(cubeMeshId);
		MeshLoader::LoadMesh("res/models/cube.mesh", cubeMesh);
		
		// Shader & material

		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.SetAllocator(&this->stackAllocator);
		colShader.LoadFromConfiguration("res/shaders/lighting.shader.json");

		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.SetShader(colShader);

		// Objects

		SceneObjectId groundSceneObj = scene.AddSceneObject();
		RenderObject& groundRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		groundRenderObj.mesh = groundMeshId;
		groundRenderObj.material = colorMaterialId;
		groundRenderObj.sceneObjectId = groundSceneObj;

		SceneObjectId sphereSceneObj = scene.AddSceneObject();
		RenderObject& sphereRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		sphereRenderObj.mesh = sphereMeshId;
		sphereRenderObj.material = colorMaterialId;
		sphereRenderObj.sceneObjectId = sphereSceneObj;
		scene.SetLocalTransform(sphereSceneObj, Matrix::Translate(Vec3f(-2.0f, 1.0f, 0.0f)));

		SceneObjectId icoSceneObj = scene.AddSceneObject();
		RenderObject& icoRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		icoRenderObj.mesh = icoMeshId;
		icoRenderObj.material = colorMaterialId;
		icoRenderObj.sceneObjectId = icoSceneObj;
		scene.SetLocalTransform(icoSceneObj, Matrix::Translate(Vec3f(0.0f, 1.0f, 0.0f)));

		SceneObjectId cubeSceneObj = scene.AddSceneObject();
		RenderObject& cubeRenderObj = renderer.GetRenderObject(renderer.AddRenderObject());
		cubeRenderObj.mesh = cubeMeshId;
		cubeRenderObj.material = colorMaterialId;
		cubeRenderObj.sceneObjectId = cubeSceneObj;
		scene.SetLocalTransform(cubeSceneObj, Matrix::Translate(Vec3f(2.0f, 1.0f, 0.0f)));

		root0 = this->scene.AddSceneObject();

		this->mainCamera.transform.position = Vec3f(0.0f, 1.0f, 8.0f);
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
	static float angle = 0.0f;

	this->time.Update();
	this->input.Update();
	this->cameraController.Update();

	TransformSource& r0t = this->scene.GetLocalTransformSource(root0);
	angle += Time::GetDeltaTime() * 0.1f;
	r0t.rotation = Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), angle);
	this->scene.SetLocalTransform(root0, r0t.GetTransformMatrix());

	this->scene.CalculateWorldTransforms();
	this->renderer.Render(this->scene);
	this->mainWindow.Swap();
}
