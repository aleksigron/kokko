#pragma once

#include "Resources/ResourceManagers.hpp"

class World;

struct Entity;
struct SceneObjectId;

namespace YAML { class Emitter; }

class LevelWriter
{
public:
	LevelWriter(World* world, const ResourceManagers& resManagers);

	bool WriteToFile(const char* filePath);

private:
	World* world;
	ResourceManagers resourceManagers;

	void WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
	void WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
	void WriteRenderComponent(YAML::Emitter& out, Entity entity);
	void WriteLightComponent(YAML::Emitter& out, Entity entity);
	void WriteCameraComponent(YAML::Emitter& out, Entity entity);
	void WriteTerrainComponent(YAML::Emitter& out, Entity entity);
	void WriteParticleComponent(YAML::Emitter& out, Entity entity);
};
