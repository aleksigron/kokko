#include "App.h"

#include "GeometryBuilder.h"
#include "ImageData.h"

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

		// First cube

		ObjectId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.Load("res/shaders/simple.vert", "res/shaders/simple.frag");

		this->cube0 = GeometryBuilder::UnitCubeWithColor();
		RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
		cube0obj.shader = colShaderId;
		cube0obj.hasTexture = false;

		// Second cube

		ObjectId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.Load("res/shaders/tex.vert", "res/shaders/tex.frag");

		this->cube1 = GeometryBuilder::UnitCubeWithTextureCoords();
		RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
		cube1obj.shader = texShaderId;
		cube1obj.texture = texId;
		cube1obj.hasTexture = true;

		this->mainCamera.position = Vec3f(0.0f, 0.0f, 5.0f);
		
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
	
	this->mainCamera.SetFrameSize(this->mainWindow.GetFrameBufferSize());
	this->renderer.Render();
	this->mainWindow.Swap();
}
