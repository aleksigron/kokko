#pragma once

#include "Core/Optional.hpp"
#include "Core/Uid.hpp"

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
	Optional<Uid> selectedAsset;

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
