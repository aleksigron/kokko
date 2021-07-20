#pragma once

#include "Core/Array.hpp"

#include "Resources/ResourceManagers.hpp"

class Allocator;
class World;
class ComponentSerializer;
class TransformSerializer;

struct Entity;
struct SceneObjectId;

namespace YAML
{
	class Emitter;
	class Node;
}

class LevelSerializer
{
public:
	LevelSerializer(Allocator* allocator);
	~LevelSerializer();

	void Initialize(World* world, const ResourceManagers& resourceManagers);

	void DeserializeFromString(const char* data);
	bool SerializeToFile(const char* filePath);

private:
	Allocator* allocator;
	World* world;
	ResourceManagers resourceManagers;

	TransformSerializer* transformSerializer;
	Array<ComponentSerializer*> componentSerializers;

	// Serialization

	void WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);

	// Deserialization

	// If parent is set to other than Null, will create children for that object
	void CreateObjects(const YAML::Node& objectSequence, SceneObjectId parent);

	SceneObjectId CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent);
};
