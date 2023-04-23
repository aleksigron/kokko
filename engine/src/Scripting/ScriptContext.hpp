#pragma once

namespace kokko
{
class World;
}

class AppSettings;
class InputManager;

struct ScriptContext
{
	void* app;
	kokko::World* world;
	InputManager* inputManager;
};
