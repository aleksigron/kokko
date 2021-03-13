#pragma once

class AppSettings;
class SceneManager;
class InputManager;

struct ScriptContext
{
	void* app;
	SceneManager* sceneManager;
	InputManager* inputManager;
};
