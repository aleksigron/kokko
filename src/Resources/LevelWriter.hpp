#pragma once

#include "Core/ArrayView.hpp"

#include "Resources/ResourceManagers.hpp"

class World;
class ComponentSerializer;

struct Entity;
struct SceneObjectId;

namespace YAML { class Emitter; }

class LevelWriter
{
public:
	LevelWriter(World* world, const ResourceManagers& resManagers, ArrayView<ComponentSerializer*> componentSerializers);

	bool WriteToFile(const char* filePath);

private:
	World* world;
	ResourceManagers resourceManagers;
	ArrayView<ComponentSerializer*> componentSerializers;

	void WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
	void WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
};
