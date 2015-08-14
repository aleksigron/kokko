#include "App.h"

#include "GeometryBuilder.h"
#include "ImageData.h"

#include <string>
#include <iostream>

#define GLFW_INCLUDE_GLCOREARB
#include "glfw/glfw3.h"

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
	if (this->mainWindow.Initialize())
	{
		this->renderer.Initialize();
		this->renderer.AttachTarget(&this->mainWindow);
		this->renderer.SetActiveCamera(&this->mainCamera);

		// Test image
		ImageData image;
		image.LoadPng("res/textures/test.png");

		ObjectId texId = this->resourceManager.textures.Add();
		Texture& tex = this->resourceManager.textures.Get(texId);
		tex.Upload(image);

		image.DeallocateData();

		// Color shader
		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.LoadFromConfiguration("res/shaders/simple.json");

		// Color material
		ObjectId colorMaterialId = resourceManager.materials.Add();
		Material& colorMaterial = resourceManager.materials.Get(colorMaterialId);
		colorMaterial.SetShader(colShader);

		// Color object
		this->cube0 = GeometryBuilder::UnitCubeWithColor();
		RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
		cube0obj.material = colorMaterialId;

		// Texture shader
		ObjectId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.LoadFromConfiguration("res/shaders/tex.json");

		// Texture material
		ObjectId texMaterialId = resourceManager.materials.Add();
		Material& texMaterial = resourceManager.materials.Get(texMaterialId);
		texMaterial.SetShader(texShader);
		texMaterial.SetUniformValue(0, tex.textureGlId);

		// Second cube

		this->cube1 = GeometryBuilder::UnitCubeWithTextureCoords();
		RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
		cube1obj.material = texMaterialId;

		this->mainCamera.transform.position = Vec3f(0.0f, 0.0f, 5.0f);

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

	float t = Time::GetTime();
	
	RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
	cube0obj.transform.position = Vec3f(cosf(t * 0.5f) * 1.2f, sinf(t * 0.5f) * 1.2f, 0.0f);
	cube0obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, 1.0f), t * -1.0f);

	RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
	cube1obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, -1.0f), t * 1.0f);
	cube1obj.transform.position = Vec3f(cosf(t * 0.5f) * -1.2f, sinf(t * 0.5f) * -1.2f, 0.0f);

	this->renderer.Render();
	this->mainWindow.Swap();
}
