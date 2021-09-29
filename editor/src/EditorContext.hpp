#pragma once

#include "Engine/Entity.hpp"

class World;

struct EngineSettings;

struct EditorContext
{
	World* world;
	EngineSettings* engineSettings;
	Entity selectedEntity;

	EditorContext() :
		world(nullptr),
		engineSettings(nullptr),
		selectedEntity(Entity::Null)
	{
	}
};
