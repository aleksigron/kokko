#include "Scene.hpp"

#include <cassert>

#include "Math.hpp"

const SceneObjectId SceneObjectId::Null = SceneObjectId { };

Scene::Scene(unsigned int sceneId) : sceneId(sceneId), activeCamera(nullptr)
{
	data = InstanceData{};

	this->Reallocate(512);
}

Scene::~Scene()
{
	operator delete[](data.buffer);
}

Scene& Scene::operator=(Scene&& other)
{
	this->sceneId = other.sceneId;
	this->data = other.data;
	this->activeCamera = other.activeCamera;

	other.sceneId = 0;
	other.data = InstanceData{}; // Zero-initialize
	other.activeCamera = nullptr;

	return *this;
}

void Scene::Reallocate(unsigned int required)
{
	if (required <= data.allocated)
		return;

	required = Math::UpperPowerOfTwo(required);

	// Reserve same amount in entity map
	map.Reserve(required);

	InstanceData newData;
	const unsigned objectBytes = sizeof(Entity) + 2 * sizeof(Mat4x4f) + 4 * sizeof(SceneObjectId);
	newData.buffer = operator new[](required * objectBytes);
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.local = reinterpret_cast<Mat4x4f*>(newData.entity + required);
	newData.world = newData.local + required;
	newData.parent = reinterpret_cast<SceneObjectId*>(newData.world + required);
	newData.firstChild = newData.parent + required;
	newData.nextSibling = newData.firstChild + required;
	newData.prevSibling = newData.nextSibling + required;

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.local, data.local, data.count * sizeof(Mat4x4f));
		std::memcpy(newData.world, data.world, data.count * sizeof(Mat4x4f));
		std::memcpy(newData.parent, data.parent, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.firstChild, data.firstChild, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.nextSibling, data.nextSibling, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.prevSibling, data.prevSibling, data.count * sizeof(SceneObjectId));

		operator delete[](data.buffer);
	}

	data = newData;
}

SceneObjectId Scene::AddSceneObject(Entity e)
{
	SceneObjectId id;
	this->AddSceneObject(1, &e, &id);
	return id;
}

void Scene::AddSceneObject(unsigned int count, Entity* entities, SceneObjectId* sceneObjectIdsOut)
{
	if (data.count + count > data.allocated)
		this->Reallocate(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = map.Insert(e.id);
		mapPair->value = SceneObjectId { id };

		data.entity[id] = e;
		data.local[id] = Mat4x4f();
		data.world[id] = Mat4x4f();
		data.parent[id] = SceneObjectId::Null;
		data.firstChild[id] = SceneObjectId::Null;
		data.nextSibling[id] = SceneObjectId::Null;
		data.prevSibling[id] = SceneObjectId::Null;

		sceneObjectIdsOut[i].i = id;
	}

	data.count += count;
}

void Scene::SetLocalTransform(SceneObjectId id, const Mat4x4f& transform)
{
	assert(id.i != SceneObjectId::Null.i);

	data.local[id.i] = transform;

	SceneObjectId parent = data.parent[id.i];

	// No check needed for invalid parent, because root index has valid transforms
	Mat4x4f parentTransform = data.world[parent.i];

	// Set world transform for specified object
	data.world[id.i] = data.local[id.i] * parentTransform;

	// Set world transforms for all children of the specified object

	SceneObjectId current = data.firstChild[id.i];
	SceneObjectId lastValid;

	while (IsValidId(current))
	{
		// Transform subtree
		data.world[id.i] = data.local[id.i] * parentTransform;

		// Move to child
		lastValid = current;
		current = data.firstChild[current.i];

		// No children for <current>
		if (IsValidId(current) == false)
		{
			// Move to next sibling
			current = data.nextSibling[lastValid.i];

			// Enter if no more siblings, find next parent's valid sibling
			// Break out when we find a valid object or hit the specified object
			while (IsValidId(current) == false && lastValid.i != id.i)
			{
				lastValid = data.parent[lastValid.i];
				current = data.nextSibling[lastValid.i];
			}
		}
	}
}
