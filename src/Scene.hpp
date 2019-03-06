#pragma once

#include "HashMap.hpp"
#include "Mat4x4.hpp"
#include "Color.hpp"
#include "Skybox.hpp"

#include "Entity.hpp"

class Camera;

struct SceneObjectId
{
	unsigned int i;

	static const SceneObjectId Null;
};

class Scene
{
private:
	struct InstanceData
	{
		unsigned int count;
		unsigned int allocated;
		void *buffer;

		Entity* entity;
		Mat4x4f* local;
		Mat4x4f* world;
		SceneObjectId* parent;
		SceneObjectId* firstChild;
		SceneObjectId* nextSibling;
		SceneObjectId* prevSibling;
	}
	data;

	HashMap<unsigned int, SceneObjectId> map;

	unsigned int sceneId;

	Camera* activeCamera;

	void Reallocate(unsigned int required);

	static bool IsValidId(SceneObjectId id) { return id.i != 0; }

public:
	Scene(unsigned int sceneId);
	~Scene();

	Scene& operator=(Scene&& other);

	void Initialize();

	Color backgroundColor;
	Skybox skybox;

	unsigned int GetSceneId() const { return sceneId; }

	SceneObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, SceneObjectId>::KeyValuePair* pair = map.Lookup(e.id);
		return pair != nullptr ? pair->value : SceneObjectId::Null;
	}

	SceneObjectId AddSceneObject(Entity e);
	void AddSceneObject(unsigned int count, Entity* entities, SceneObjectId* sceneObjectIds);

	void SetParent(SceneObjectId id, SceneObjectId parent)
	{
		data.parent[id.i] = parent;
	}

	void SetLocalTransform(SceneObjectId id, const Mat4x4f& transform);

	const Mat4x4f& GetWorldTransform(SceneObjectId id) { return data.world[id.i]; }
	const Mat4x4f& GetLocalTransform(SceneObjectId id) { return data.local[id.i]; }

	void SetActiveCamera(Camera* camera) { activeCamera = camera; }
	Camera* GetActiveCamera() { return activeCamera; }
};
