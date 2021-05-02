#include "App.hpp"

#include "Engine/Engine.hpp"
#include "Graphics/World.hpp"

App::App(Engine* engine, Allocator* allocator) :
	engine(engine),
	allocator(allocator),
	settings(allocator)
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
