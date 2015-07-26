#include "App.h"

#include "GeometryBuilder.h"

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

		// First cube

		ShaderProgramId colShaderId = resourceManager.shaders.Add();
		ShaderProgram& colShader = resourceManager.shaders.Get(colShaderId);
		colShader.Load("res/shaders/simple.vert", "res/shaders/simple.frag");

		this->cube0 = GeometryBuilder::UnitCubeWithColor();
		RenderObject& cube0obj = this->renderer.GetRenderObject(this->cube0);
		cube0obj.shader = colShaderId;

		// Second cube

		ShaderProgramId texShaderId = resourceManager.shaders.Add();
		ShaderProgram& texShader = resourceManager.shaders.Get(texShaderId);
		texShader.Load("res/shaders/tex.vert", "res/shaders/tex.frag");

		this->cube1 = GeometryBuilder::UnitCubeWithTextureCoords();
		RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
		cube1obj.shader = texShaderId;

		this->mainCamera.position = Vec3f(0.0f, 0.0f, 8.0f);
		
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
	cube0obj.transform.position = Vec3f(cosf(t * 0.5f) * 2.0f, sinf(t * 0.5f) * 2.0f, 0.0f);
	cube0obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, 1.0f), t * -1.5f);

	RenderObject& cube1obj = this->renderer.GetRenderObject(this->cube1);
	cube1obj.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, -1.0f), t * 1.5f);
	cube1obj.transform.position = Vec3f(cosf(t * 0.5f) * -2.0f, sinf(t * 0.5f) * -2.0f, 0.0f);
	
	this->mainCamera.SetFrameSize(this->mainWindow.GetFrameBufferSize());
	this->renderer.Render();
	this->mainWindow.Swap();
}
