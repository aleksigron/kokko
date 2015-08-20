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

	if (this->mainWindow.Initialize())
	{
		this->input.Initialize();
		this->renderer.Initialize();
		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);
		this->cameraController.SetControlledCamera(&this->mainCamera);

		// Test image
		ImageData image;
		image.LoadPng("res/textures/test.png");

		ObjectId texId = this->resourceManager.textures.Add();
		Texture& tex = this->resourceManager.textures.Get(texId);
		tex.Upload(image);

		image.DeallocateData();

		// Color material

		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.LoadFromConfiguration("res/shaders/simple.shader.json");

		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.SetShader(colShader);

		// Texture material

		ObjectId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.LoadFromConfiguration("res/shaders/tex.shader.json");

		ObjectId texMaterialId = resourceManager.materials.Add();
		Material& texMaterial = resourceManager.materials.Get(texMaterialId);
		texMaterial.SetShader(texShader);
		texMaterial.SetUniformValue(0, tex.textureGlId);

		for (unsigned i = 0; i < 512; ++i)
		{
			ObjectId objId = GeometryBuilder::UnitCubeWithColor();
			RenderObject& obj = this->renderer.GetRenderObject(objId);
			obj.material = i < 256 ? colorMaterialId : texMaterialId;

			obj.transform.position.x = -7.0f + (i / 64 * 2.0f);
			obj.transform.position.y = -7.0f + (i % 8 * 2.0f);
			obj.transform.position.z = -7.0f + (i / 8 % 8 * 2.0f);
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
	this->time.Update();
	this->input.Update();
	this->cameraController.Update();

	this->renderer.Render();
	this->mainWindow.Swap();
}
