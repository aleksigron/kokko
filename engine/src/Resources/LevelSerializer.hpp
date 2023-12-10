#pragma once

#include "Core/Array.hpp"
#include "Core/ArrayView.hpp"
#include "Core/StringView.hpp"

#include "Resources/ResourceManagers.hpp"

class Allocator;
class ComponentSerializer;
class TransformSerializer;

struct Entity;
struct SceneObjectId;

namespace c4
{
namespace yml
{
class ConstNodeRef;
class NodeRef;
}
}

namespace kokko
{
class String;
class World;

namespace render
{
class Device;
}

class LevelSerializer
{
public:
	LevelSerializer(Allocator* allocator, kokko::render::Device* renderDevice);
	~LevelSerializer();

	void Initialize(kokko::World* world, const kokko::ResourceManagers& resourceManagers);

	void DeserializeFromString(MutableStringView data);
	void SerializeToString(kokko::String& out);

	void DeserializeEntitiesFromString(ConstStringView data, SceneObjectId parent);
	void SerializeEntitiesToString(ArrayView<Entity> serializeEntities, kokko::String& serializedOut);

private:
	Allocator* allocator;
	kokko::render::Device* renderDevice;
	kokko::World* world;
	kokko::ResourceManagers resourceManagers;

	TransformSerializer* transformSerializer;
	Array<ComponentSerializer*> componentSerializers;

	// Serialization
	void WriteEntity(c4::yml::NodeRef& entitySeq, Entity entity, SceneObjectId sceneObj);

	// Deserialization

	// If parent is set to other than Null, will create children for that object
	void CreateObjects(const c4::yml::ConstNodeRef& objectSeq, SceneObjectId parent);

	SceneObjectId CreateComponents(const c4::yml::ConstNodeRef& componentSeq, Entity entity, SceneObjectId parent);
};

} // namespace kokko
