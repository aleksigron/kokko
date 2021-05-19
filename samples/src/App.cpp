#include "App.hpp"

#include "Engine/Engine.hpp"

App::App(Engine* engine, Allocator* allocator) :
	engine(engine),
	allocator(allocator)
{
}

App::~App()
{
}

void App::Initialize()
{
}

void App::Update()
{
}
