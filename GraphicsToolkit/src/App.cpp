#include "App.h"

#include "GeometryBuilder.h"
#include "ImageData.h"

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

		// Test image
		ImageData rawImage;
		rawImage.LoadGlraw("res/textures/test.glraw");

		ObjectId texId = this->resourceManager.textures.Add();
		Texture& tex = this->resourceManager.textures.Get(texId);
		tex.Upload(rawImage);

		ImageData compressedImage;
		compressedImage.LoadGlraw("res/textures/concrete.dxt1.glraw");

		ObjectId compressedTexId = this->resourceManager.textures.Add();
		Texture& compressedTex = this->resourceManager.textures.Get(compressedTexId);
		compressedTex.Upload(compressedImage);

		// Color material

		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.SetAllocator(&this->stackAllocator);
		colShader.LoadFromConfiguration("res/shaders/simple.shader.json");

		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.SetShader(colShader);

		// Texture material

		ObjectId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.SetAllocator(&this->stackAllocator);
		texShader.LoadFromConfiguration("res/shaders/tex.shader.json");

		ObjectId texMaterialId = resourceManager.materials.Add();
		Material& texMaterial = resourceManager.materials.Get(texMaterialId);
		texMaterial.SetShader(texShader);
		texMaterial.SetUniformValue(0, compressedTex.textureGlId);

		ObjectId colorCubeMeshId = GeometryBuilder::UnitCubeWithColor();
		ObjectId textureCubeMeshId = GeometryBuilder::UnitCubeWithTextureCoords();

		root0 = this->scene.AddSceneObject();
		root1 = this->scene.AddSceneObject();

		for (unsigned i = 0; i < 64; ++i)
		{
			ObjectId objId = this->renderer.AddRenderObject();
			RenderObject& obj = this->renderer.GetRenderObject(objId);
			obj.mesh = i < 32 ? colorCubeMeshId : textureCubeMeshId;
			obj.material = i < 32 ? colorMaterialId : texMaterialId;

			obj.sceneObjectId = this->scene.AddSceneObject();
			this->scene.SetParent(obj.sceneObjectId, i < 32 ? root0 : root1);

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
	TransformSource& r1t = this->scene.GetLocalTransformSource(root1);

	angle += Time::GetDeltaTime() * 0.1f;

	r0t.rotation = Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), angle);
	r1t.rotation = Mat3x3f::RotateAroundAxis(Vec3f(1.0f, 0.0f, 0.0f), -angle);

	this->scene.SetLocalTransform(root0, r0t.GetTransformMatrix());
	this->scene.SetLocalTransform(root1, r1t.GetTransformMatrix());

	this->scene.CalculateWorldTransforms();
	this->renderer.Render(this->scene);
	this->mainWindow.Swap();
}
