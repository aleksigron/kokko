#pragma once

#include "rapidjson/document.h"

#include "Core/BufferRef.hpp"

class Engine;
class World;
class Renderer;
class MeshManager;
class MaterialManager;
class EntityManager;
class ResourceManager;
class LightManager;
class EnvironmentManager;

struct Entity;
struct SceneObjectId;

class SceneLoader
{
private:
	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	World* world;
	Renderer* renderer;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	EntityManager* entityManager;
	LightManager* lightManager;
	EnvironmentManager* environmentManager;

	void CreateObjects(ValueItr begin, ValueItr end);
	void CreateChildObjects(ValueItr begin, ValueItr end, SceneObjectId parent);
	void CreateSceneObject(ValueItr itr, SceneObjectId sceneObject);

	void CreateComponents(ValueItr itr, ValueItr end, Entity entity);
	void CreateRenderObject(ValueItr itr, Entity entity);
	void CreateLight(ValueItr itr, Entity entity);

public:
	SceneLoader(Engine* engine, World* world);

	void Load(BufferRef<char> sceneConfig);
};
