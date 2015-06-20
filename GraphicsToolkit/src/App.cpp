#include "App.h"

App::App()
{
}

App::~App()
{
	
}

bool App::Initialize()
{
	return this->mainWindow.Initialize();
}

bool App::HasRequestedQuit()
{
	return this->mainWindow.ShouldClose();
}

void App::Update()
{
	this->mainWindow.TestDraw();
}