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

		ShaderProgramId shaderId = shaderManager.shaders.Add();
		ShaderProgram& shader = shaderManager.shaders.Get(shaderId);
		shader.Load("res/shaders/simple.vert", "res/shaders/simple.frag");

		this->testCube = GeometryBuilder::UnitCubeWithColor();
		RenderObject& cube = this->renderer.GetRenderObject(this->testCube);
		cube.shader = shaderId;
		
		this->mainCamera.position = Vec3f(0.0f, 0.0f, 2.0f);
		
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
	
	this->mainCamera.position = Vec3f(cosf(Time::GetTime() * 0.5f) * 2.0f, sinf(Time::GetTime() * 0.5f) * 2.0f, 8.0f);
	
	RenderObject& cube = this->renderer.GetRenderObject(testCube);
	cube.transform.rotation = Matrix::Rotate(Vec3f(1.0f, 1.0f, 1.0f), Time::GetTime() * -1.5f);
	
	this->mainCamera.SetFrameSize(this->mainWindow.GetFrameBufferSize());
	this->renderer.Render();
	this->mainWindow.Swap();
}
