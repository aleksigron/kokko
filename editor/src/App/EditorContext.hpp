#pragma once

#include "Core/Optional.hpp"
#include "Core/Uid.hpp"
#include "Core/String.hpp"

#include "Engine/Entity.hpp"

struct ImFont;


namespace kokko
{

class Allocator;
class AssetLibrary;
class FilesystemResolver;
class World;

struct EngineSettings;

namespace editor
{

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

	ImFont* monospaceFont;

	String temporaryString;

	explicit EditorContext(Allocator* allocator, FilesystemResolver* filesystemResolver) :
		project(nullptr),
		assetLibrary(nullptr),
		filesystemResolver(filesystemResolver),
		world(nullptr),
		engineSettings(nullptr),
		selectedEntity(Entity::Null),
		monospaceFont(nullptr),
		temporaryString(allocator)
	{
	}
};

}
}
