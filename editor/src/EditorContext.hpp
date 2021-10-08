#pragma once

#include "Engine/Entity.hpp"

class World;

struct EngineSettings;

namespace kokko
{
namespace editor
{

class EditorProject;

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

}
}
