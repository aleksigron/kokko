#pragma once

#include "rapidjson/document.h"

#include "Core/BufferRef.hpp"

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

namespace YAML { class Node; }

class LevelLoader
{
private:
	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	EntityManager* entityManager;
	World* world;
	Renderer* renderer;
	LightManager* lightManager;
	CameraSystem* cameraSystem;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	EnvironmentManager* environmentManager;

	// If parent is set to other than Null, will create children for that object
	void CreateObjects(const YAML::Node& objectSequence, SceneObjectId parent);

	SceneObjectId CreateComponents(const YAML::Node& componentSequence, Entity entity, SceneObjectId parent);

	SceneObjectId CreateTransformComponent(const YAML::Node& map, Entity entity, SceneObjectId parent);
	void CreateRenderComponent(const YAML::Node& map, Entity entity);
	void CreateLightComponent(const YAML::Node& map, Entity entity);
	void CreateCameraComponent(const YAML::Node& map, Entity entity);

	void CreateObjects(ValueItr begin, ValueItr end);
	void CreateChildObjects(ValueItr begin, ValueItr end, SceneObjectId parent);
	void CreateSceneObject(ValueItr itr, SceneObjectId sceneObject);

	void CreateComponents(ValueItr itr, ValueItr end, Entity entity);
	void CreateRenderObject(ValueItr itr, Entity entity);
	void CreateLight(ValueItr itr, Entity entity);

public:
	LevelLoader(Engine* engine);

	void Load(BufferRef<char> sceneConfig);
	void LoadJson(BufferRef<char> sceneConfig);
};
