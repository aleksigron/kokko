#pragma once

#include "Core/HashMap.hpp"
#include "Core/Array.hpp"
#include "Core/SortedArray.hpp"

#include "Entity/Entity.hpp"

#include "Math/Mat4x4.hpp"

class Camera;
class Allocator;
class TransformUpdateReceiver;

struct StringRef;

struct SceneObjectId
{
	unsigned int i;

	bool operator==(SceneObjectId other) { return i == other.i; }
	bool operator!=(SceneObjectId other) { return !operator==(other); }

	static const SceneObjectId Null;
};

struct SceneEditTransform
{
	Vec3f translation;
	Vec3f rotation;
	Vec3f scale;

	SceneEditTransform() : scale(1.0f, 1.0f, 1.0f)
	{
	}

	SceneEditTransform(const Vec3f& pos) :
		translation(pos),
		scale(1.0f, 1.0f, 1.0f)
	{
	}
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
		SceneEditTransform* editTransform;
	}
	data;

	HashMap<unsigned int, SceneObjectId> entityMap;
	SortedArray<unsigned int> updatedEntities;
	Array<Mat4x4f> updatedTransforms;

	int environmentId;

	Entity activeCamera;

	void Reallocate(unsigned int required);

	// The specified object and all its children are updated
	// The world transform is calculated and the objects are marked as updated
	void UpdateWorldTransforms(SceneObjectId id);

public:
	Scene(Allocator* allocator);
	Scene(const Scene& other) = delete;
	Scene(Scene&& other) = delete;
	~Scene();

	Scene& operator=(const Scene& other) = delete;
	Scene& operator=(Scene&& other) = delete;

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

	void Clear();

	Entity GetEntity(SceneObjectId id) const { return data.entity[id.i]; }
	SceneObjectId GetParent(SceneObjectId id) const { return data.parent[id.i]; }
	SceneObjectId GetFirstChild(SceneObjectId id) const { return data.firstChild[id.i]; }
	SceneObjectId GetNextSibling(SceneObjectId id) const { return data.nextSibling[id.i]; }

	void SetParent(SceneObjectId id, SceneObjectId parent);

	void SetLocalTransform(SceneObjectId id, const Mat4x4f& transform);

	const Mat4x4f& GetWorldTransform(SceneObjectId id) { return data.world[id.i]; }
	const Mat4x4f& GetLocalTransform(SceneObjectId id) { return data.local[id.i]; }

	const SceneEditTransform& GetEditTransform(SceneObjectId id);
	void SetEditTransform(SceneObjectId id, const SceneEditTransform& editTransform);

	// This can be used to manually add the entity to the updated transforms queue.
	// This is useful when the transform hasn't changed but other relevant info,
	// such as the bounding box has changed.
	// Only the specified object is marked, but not its children
	void MarkUpdated(SceneObjectId id);

	void NotifyUpdatedTransforms(unsigned int receiverCount, TransformUpdateReceiver** updateReceivers);

	void SetEnvironmentId(int environmentId) { this->environmentId = environmentId; }
	int GetEnvironmentId() const { return this->environmentId; }

	void SetActiveCameraEntity(Entity camera) { activeCamera = camera; }
	Entity GetActiveCameraEntity() const { return activeCamera; }
};
