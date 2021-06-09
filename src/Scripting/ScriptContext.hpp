#pragma once

class AppSettings;
class Scene;
class InputManager;

struct ScriptContext
{
	void* app;
	Scene* scene;
	InputManager* inputManager;
};
