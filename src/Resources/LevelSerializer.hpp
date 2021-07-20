#pragma once

#include "Core/Array.hpp"

#include "Resources/ResourceManagers.hpp"

class Allocator;
class World;
class ComponentSerializer;

namespace YAML { class Node; }

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

	Array<ComponentSerializer*> componentSerializers;
};
