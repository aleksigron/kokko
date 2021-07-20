#pragma once

#include "Core/ArrayView.hpp"

#include "Resources/ResourceManagers.hpp"

class World;
class ComponentSerializer;

struct Entity;
struct SceneObjectId;

namespace YAML { class Node; }

class LevelLoader
{
private:
	World* world;
	ResourceManagers resourceManagers;
	ArrayView<ComponentSerializer*> componentSerializers;

	// If parent is set to other than Null, will create children for that object
	void CreateObjects(const YAML::Node& objectSequence, SceneObjectId parent);

	SceneObjectId CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent);

	SceneObjectId CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent);

public:
	LevelLoader(World* world, const ResourceManagers& resManagers, ArrayView<ComponentSerializer*> componentSerializers);

	void Load(const char* data);
};
