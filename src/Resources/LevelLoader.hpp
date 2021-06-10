#pragma once

#include "Core/BufferRef.hpp"

#include "Resources/ResourceManagers.hpp"

class World;

struct Entity;
struct SceneObjectId;

namespace YAML { class Node; }

class LevelLoader
{
private:
	World* world;
	ResourceManagers resourceManagers;

	// If parent is set to other than Null, will create children for that object
	void CreateObjects(const YAML::Node& objectSequence, SceneObjectId parent);

	SceneObjectId CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent);

	SceneObjectId CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent);
	void CreateRenderComponent(const YAML::Node& map, Entity entity);
	void CreateLightComponent(const YAML::Node& map, Entity entity);
	void CreateCameraComponent(const YAML::Node& map, Entity entity);

public:
	LevelLoader(World* world, const ResourceManagers& resManagers);

	void Load(BufferRef<char> sceneConfig);
};
