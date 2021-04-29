#pragma once

class AppSettings;
class Scene;
class InputManager;

struct ScriptContext
{
	void* app;
	Scene* world;
	InputManager* inputManager;
};
