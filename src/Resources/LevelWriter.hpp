#pragma once

class Engine;
class EntityManager;
class World;
class Renderer;
class LightManager;
class CameraSystem;
class MeshManager;
class MaterialManager;
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
	EntityManager* entityManager;
	World* world;
	Renderer* renderer;
	LightManager* lightManager;
	CameraSystem* cameraSystem;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	EnvironmentManager* environmentManager;

	void WriteEntity(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
	void WriteTransformComponent(YAML::Emitter& out, Entity entity, SceneObjectId sceneObj);
	void WriteRenderComponent(YAML::Emitter& out, Entity entity);
	void WriteLightComponent(YAML::Emitter& out, Entity entity);
	void WriteCameraComponent(YAML::Emitter& out, Entity entity);
};
