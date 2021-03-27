#include "Scene/Scene.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Memory/Allocator.hpp"
#include "Math/Math.hpp"
#include "ITransformUpdateReceiver.hpp"

const SceneObjectId SceneObjectId::Null = SceneObjectId{};

Scene::Scene(Allocator* allocator, unsigned int sceneId):
	allocator(allocator),
	entityMap(allocator),
	updatedEntities(allocator),
	updatedTransforms(allocator),
	sceneId(sceneId),
	environmentId(-1),
	activeCamera(nullptr)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as SceneObjectId::Null value

	this->Reallocate(512);
}

Scene::~Scene()
{
	allocator->Deallocate(data.buffer);
}

Scene& Scene::operator=(Scene&& other) noexcept
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
	entityMap.Reserve(required);

	InstanceData newData;
	const unsigned objectBytes = sizeof(Entity) + 2 * sizeof(Mat4x4f) + 4 * sizeof(SceneObjectId);
	newData.buffer = allocator->Allocate(required * objectBytes);
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

		allocator->Deallocate(data.buffer);
	}
	else
	{
		newData.entity[SceneObjectId::Null.i] = Entity{};
		newData.local[SceneObjectId::Null.i] = Mat4x4f();
		newData.world[SceneObjectId::Null.i] = Mat4x4f();
		newData.parent[SceneObjectId::Null.i] = SceneObjectId::Null;
		newData.firstChild[SceneObjectId::Null.i] = SceneObjectId::Null;
		newData.nextSibling[SceneObjectId::Null.i] = SceneObjectId::Null;
		newData.prevSibling[SceneObjectId::Null.i] = SceneObjectId::Null;
	}

	data = newData;
}

void Scene::AddSceneObject(unsigned int count, Entity* entities, SceneObjectId* idsOut)
{
	if (data.count + count > data.allocated)
		this->Reallocate(data.count + count);

	for (unsigned int i = 0; i < count; ++i)
	{
		unsigned int id = data.count + i;

		Entity e = entities[i];

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second = SceneObjectId { id };

		data.entity[id] = e;
		data.local[id] = Mat4x4f();
		data.world[id] = Mat4x4f();
		data.parent[id] = SceneObjectId::Null;
		data.firstChild[id] = SceneObjectId::Null;
		data.nextSibling[id] = SceneObjectId::Null;
		data.prevSibling[id] = SceneObjectId::Null;

		idsOut[i].i = id;
	}

	data.count += count;

	updatedEntities.InsertUnique(reinterpret_cast<unsigned int*>(entities), count);
}

void Scene::RemoveSceneObject(SceneObjectId id)
{
	assert(IsValidId(id));

	// Remove references
	{
		SceneObjectId parent = data.parent[id.i];
		SceneObjectId prevSibling = data.prevSibling[id.i];
		SceneObjectId nextSibling = data.nextSibling[id.i];

		if (IsValidId(prevSibling)) // We're not the first sibling
		{
			// nextSibling can be Null, no need to check
			data.nextSibling[prevSibling.i] = nextSibling;
		}
		else if (IsValidId(parent)) // We have a parent that's not the root
		{
			// Because we didn't have prevSibling, we know we were the first child
			// nextSibling can be Null, no need to check
			data.firstChild[parent.i] = nextSibling;
		}

		if (IsValidId(nextSibling)) // We have nextSibling, its prevSibling must be updated
		{
			// prevSibling can be Null, no need to check
			data.prevSibling[nextSibling.i] = prevSibling;
		}
	}

	// Swap last item in the removed object's place

	if (data.count > 1) // We still have objects other than the root
	{
		SceneObjectId swap;
		swap.i = data.count - 1;

		SceneObjectId parent = data.parent[swap.i];
		SceneObjectId firstChild = data.firstChild[swap.i];
		SceneObjectId prevSibling = data.prevSibling[swap.i];
		SceneObjectId nextSibling = data.nextSibling[swap.i];

		// Update references pointing to the swap object

		// Swap isn't the first sibling
		if (IsValidId(prevSibling))
			data.nextSibling[prevSibling.i] = id;

		// Swap has a parent that's not the root
		// Because we didn't have prevSibling, we know we were the first child
		else if (IsValidId(parent))
			data.firstChild[parent.i] = id;

		// Swap has nextSibling, its prevSibling must be updated
		if (IsValidId(nextSibling))
			data.prevSibling[nextSibling.i] = id;

		// Swap has children, their parent must be updated
		for (SceneObjectId child = firstChild; IsValidId(child); child = data.nextSibling[child.i])
			data.parent[child.i] = id;

		// Update swap objects data to the removed objects place

		data.entity[id.i] = data.entity[swap.i];
		data.local[id.i] = data.local[swap.i];
		data.world[id.i] = data.world[swap.i];
		data.parent[id.i] = parent;
		data.firstChild[id.i] = firstChild;
		data.prevSibling[id.i] = prevSibling;
		data.nextSibling[id.i] = nextSibling;

		--data.count;
	}
}

void Scene::SetParent(SceneObjectId id, SceneObjectId parent)
{
	assert(IsValidId(id));

	SceneObjectId oldParent = data.parent[id.i];

	// Check that the new parent is different from old parent
	if (oldParent.i != parent.i)
	{
		// Patch references relating to old position in hierarchy
		{
			SceneObjectId prevSibling = data.prevSibling[id.i];
			SceneObjectId nextSibling = data.nextSibling[id.i];

			// No need to check for Null reference values as they are valid

			if (IsValidId(prevSibling)) // We're not the first sibling
				data.nextSibling[prevSibling.i] = nextSibling;

			// Object has a parent that's not the root
			// Because object didn't have prevSibling, it must be the first child
			else if (IsValidId(oldParent))
				data.firstChild[oldParent.i] = nextSibling;

			// Object has nextSibling, its prevSibling must be updated
			if (IsValidId(nextSibling))
				data.prevSibling[nextSibling.i] = prevSibling;

			// No need to check for children of object
			// The SceneObjectId didn't change, so children will still be children
		}

		// Create references for new position in hierarchy

		if (IsValidId(parent)) // New parent isn't root
		{
			SceneObjectId parentsChild = data.firstChild[parent.i];

			// If the new parent has a child, set this object as the prevSibling
			if (IsValidId(parentsChild))
				data.prevSibling[parentsChild.i] = id;

			// Set this object as the first child of the new parent
			data.firstChild[parent.i] = id;
		}

		// Finally set the new parent
		data.parent[id.i] = parent;
	}
}

void Scene::SetLocalTransform(SceneObjectId id, const Mat4x4f& transform)
{
	assert(IsValidId(id));

	data.local[id.i] = transform;

	SceneObjectId parent = data.parent[id.i];

	// No check needed for invalid parent, because root index has valid transforms
	Mat4x4f parentTransform = data.world[parent.i];

	// Set world transform for specified object
	data.world[id.i] = data.local[id.i] * parentTransform;

	// Set the entity as updated
	updatedEntities.InsertUnique(data.entity[id.i].id);

	// Set world transforms for all children of the specified object

	SceneObjectId current = data.firstChild[id.i];
	SceneObjectId lastValid;

	while (IsValidId(current))
	{
		data.world[id.i] = data.local[id.i] * parentTransform;

		// Set the entity as updated
		updatedEntities.InsertUnique(data.entity[id.i].id);

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

void Scene::NotifyUpdatedTransforms(unsigned int receiverCount, ITransformUpdateReceiver** updateReceivers)
{
	KOKKO_PROFILE_FUNCTION();

	unsigned int updateCount = updatedEntities.GetCount();
	updatedTransforms.Resize(updateCount);

	Entity* entities = reinterpret_cast<Entity*>(updatedEntities.GetData());

	for (unsigned int i = 0; i < updateCount; ++i)
	{
		SceneObjectId obj = this->Lookup(entities[i]);
		updatedTransforms[i] = this->GetWorldTransform(obj);
	}

	for (unsigned int i = 0; i < receiverCount; ++i)
	{
		updateReceivers[i]->NotifyUpdatedTransforms(updateCount, entities, updatedTransforms.GetData());
	}

	updatedEntities.Clear();
	updatedTransforms.Clear();
}
