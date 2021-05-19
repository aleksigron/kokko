#pragma once

class Allocator;
class Engine;

class App
{
private:
	Engine* engine;
	Allocator* allocator;

public:
	App(Engine* engine, Allocator* allocator);
	~App();
	
	void Initialize();
	void Update();
};
