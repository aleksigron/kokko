#pragma once

#include "Application/AppSettings.hpp"

class Allocator;
class Engine;

class App
{
private:
	Engine* engine;
	Allocator* allocator;
	AppSettings settings;

public:
	App(Engine* engine, Allocator* allocator);
	~App();
	
	void Initialize();
	void Update();

	AppSettings* GetSettings() { return &settings; }
};
