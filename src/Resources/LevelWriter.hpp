#pragma once

class Engine;
class World;
class Renderer;
class MeshManager;
class MaterialManager;
class EntityManager;
class LightManager;
class EnvironmentManager;

struct Entity;
struct SceneObjectId;

namespace YAML { class Emitter; }

class LevelWriter
{
public:
	LevelWriter(Engine* engine);

	bool WriteToFile(const char* filePath);

private:
	World* world;
	Renderer* renderer;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	EntityManager* entityManager;
	LightManager* lightManager;
	EnvironmentManager* environmentManager;

	void SerializeEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
};
