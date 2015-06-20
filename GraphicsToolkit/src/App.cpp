#include "App.h"

#include "RenderObject.h"

App::App()
{
}

App::~App()
{
}

bool App::Initialize()
{
	if (this->mainWindow.Initialize())
	{
		this->renderer.AttachTarget(&this->mainWindow);
		
		Buffer<Vec3f> vertexData;
		vertexData.Allocate(3);
		vertexData[0] = Vec3f(-1.0f, -1.0f, 0.0f);
		vertexData[1] = Vec3f(1.0f, -1.0f, 0.0f);
		vertexData[2] = Vec3f(0.0f,  1.0f, 0.0f);
		
		RenderObject& obj = this->renderer.CreateRenderObject();
		obj.SetVertexBufferData(vertexData);
		
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
	this->renderer.Render();
	this->mainWindow.Swap();
}