#pragma once

namespace kokko
{
class World;

class AppSettings;
class InputManager;

struct ScriptContext
{
	void* app;
	World* world;
	InputManager* inputManager;
};

} // namespace kokko
