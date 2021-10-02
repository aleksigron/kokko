#pragma once

#include "Engine/Entity.hpp"

class World;
class EditorProject;

struct EngineSettings;

struct EditorContext
{
	const EditorProject* project;

	World* world;
	EngineSettings* engineSettings;
	Entity selectedEntity;

	EditorContext() :
		project(nullptr),
		world(nullptr),
		engineSettings(nullptr),
		selectedEntity(Entity::Null)
	{
	}
};
