#pragma once

#include "Core/Optional.hpp"
#include "Core/Uid.hpp"
#include "Core/String.hpp"

#include "Engine/Entity.hpp"

class Allocator;
class World;

namespace kokko
{

class FilesystemResolver;

struct EngineSettings;

namespace editor
{

class AssetLibrary;
class EditorProject;

struct EditorContext
{
	const EditorProject* project;
	AssetLibrary* assetLibrary;
	FilesystemResolver* filesystemResolver;

	World* world;
	EngineSettings* engineSettings;
	Entity selectedEntity;

	Optional<Uid> loadedLevel;
	Optional<Uid> requestLoadLevel;

	Optional<Uid> selectedAsset;
	Optional<Uid> editingAsset;

	String temporaryString;

	explicit EditorContext(Allocator* allocator, FilesystemResolver* filesystemResolver) :
		project(nullptr),
		assetLibrary(nullptr),
		filesystemResolver(filesystemResolver),
		world(nullptr),
		engineSettings(nullptr),
		selectedEntity(Entity::Null),
		temporaryString(allocator)
	{
	}
};

}
}
