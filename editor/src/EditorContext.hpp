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

class AssetLibrary;
class EditorProject;

struct EditorContext
{
	const EditorProject* project;
	AssetLibrary* assetLibrary;

	World* world;
	EngineSettings* engineSettings;
	Entity selectedEntity;

	Optional<Uid> selectedAsset;
	Optional<Uid> editingAsset;

	EditorContext() :
		project(nullptr),
		assetLibrary(nullptr),
		world(nullptr),
		engineSettings(nullptr),
		selectedEntity(Entity::Null)
	{
	}
};

}
}
