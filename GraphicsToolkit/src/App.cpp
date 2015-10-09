#include "App.h"

#include "GeometryBuilder.h"
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

		//ObjectId meshId = GeometryBuilder::UnitCubeWithColor();


		// Mesh from file

		ObjectId meshId = this->resourceManager.meshes.Add();
		Mesh& mesh = this->resourceManager.meshes.Get(meshId);
		MeshLoader::LoadMesh("res/models/test.mesh", mesh);
		
		// Color material

		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.SetAllocator(&this->stackAllocator);
		colShader.LoadFromConfiguration("res/shaders/lighting.shader.json");

		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.SetShader(colShader);

		root0 = this->scene.AddSceneObject();

		for (unsigned i = 0; i < 64; ++i)
		{
			ObjectId objId = this->renderer.AddRenderObject();
			RenderObject& obj = this->renderer.GetRenderObject(objId);
			obj.mesh = meshId;
			obj.material = colorMaterialId;

			obj.sceneObjectId = this->scene.AddSceneObject();
			this->scene.SetParent(obj.sceneObjectId, root0);

			Vec3f pos(-3.0f + (i / 16 * 2.0f), -3.0f + (i % 4 * 2.0f), -3.0f + (i / 4 % 4 * 2.0f));

			this->scene.SetLocalTransform(obj.sceneObjectId, Matrix::Translate(pos));
		}

		this->mainCamera.transform.position = Vec3f(0.0f, 0.0f, 15.0f);
		this->mainCamera.perspectiveFieldOfView = Mathf::DegreesToRadians(50.0f);

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
