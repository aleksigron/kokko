#pragma once

class AppSettings;
class World;
class InputManager;

struct ScriptContext
{
	void* app;
	World* world;
	InputManager* inputManager;
};
