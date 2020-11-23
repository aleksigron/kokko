#pragma once

#include "rapidjson/document.h"

#include "Core/BufferRef.hpp"
#include "Scene/Scene.hpp"

class Engine;
class Renderer;
class MeshManager;
class MaterialManager;
class EntityManager;
class ResourceManager;
class LightManager;

class SceneLoader
{
private:
	using ValueItr = rapidjson::Value::ConstValueIterator;
	using MemberItr = rapidjson::Value::ConstMemberIterator;

	Scene* scene;
	Renderer* renderer;
	MeshManager* meshManager;
	MaterialManager* materialManager;
	EntityManager* entityManager;
	LightManager* lightManager;

	void CreateObjects(ValueItr begin, ValueItr end);
	void CreateChildObjects(ValueItr begin, ValueItr end, SceneObjectId parent);
	void CreateSceneObject(ValueItr itr, SceneObjectId sceneObject);

	void CreateComponents(ValueItr itr, ValueItr end, Entity entity);
	void CreateRenderObject(ValueItr itr, Entity entity);
	void CreateLight(ValueItr itr, Entity entity);

public:
	SceneLoader(Engine* engine, Scene* scene);

	void Load(BufferRef<char> sceneConfig);
};
