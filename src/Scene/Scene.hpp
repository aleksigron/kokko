#pragma once

#include "Core/HashMap.hpp"
#include "Core/Array.hpp"
#include "Core/SortedArray.hpp"
#include "Core/Color.hpp"

#include "Entity/Entity.hpp"

#include "Math/Mat4x4.hpp"

#include "Resources/MaterialData.hpp"

class Camera;
class Allocator;
class ITransformUpdateReceiver;

struct SceneObjectId
{
	unsigned int i;

	static const SceneObjectId Null;
};

class Scene
{
private:
	Allocator* allocator;

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

	HashMap<unsigned int, SceneObjectId> entityMap;
	SortedArray<unsigned int> updatedEntities;
	Array<Mat4x4f> updatedTransforms;

	unsigned int sceneId;

	MaterialId skyboxMaterial;

	Camera* activeCamera;

	void Reallocate(unsigned int required);

	static bool IsValidId(SceneObjectId id) { return id.i != 0; }

public:
	Scene(Allocator* allocator, unsigned int sceneId);
	~Scene();

	Scene& operator=(Scene&& other);

	Color ambientColor;

	unsigned int GetSceneId() const { return sceneId; }

	SceneObjectId Lookup(Entity e)
	{
		HashMap<unsigned int, SceneObjectId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : SceneObjectId::Null;
	}

	SceneObjectId AddSceneObject(Entity e)
	{
		SceneObjectId id;
		this->AddSceneObject(1, &e, &id);
		return id;
	}

	void AddSceneObject(unsigned int count, Entity* entities, SceneObjectId* idsOut);

	void RemoveSceneObject(SceneObjectId id);

	void SetParent(SceneObjectId id, SceneObjectId parent);

	void SetLocalTransform(SceneObjectId id, const Mat4x4f& transform);

	const Mat4x4f& GetWorldTransform(SceneObjectId id) { return data.world[id.i]; }
	const Mat4x4f& GetLocalTransform(SceneObjectId id) { return data.local[id.i]; }

	void NotifyUpdatedTransforms(unsigned int receiverCount, ITransformUpdateReceiver** updateReceivers);

	void SetSkyboxMaterial(MaterialId materialId) { skyboxMaterial = materialId; }
	MaterialId GetSkyboxMaterial() const { return skyboxMaterial; }

	void SetActiveCamera(Camera* camera) { activeCamera = camera; }
	Camera* GetActiveCamera() { return activeCamera; }
};
