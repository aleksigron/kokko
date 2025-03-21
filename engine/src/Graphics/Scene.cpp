#include "Graphics/Scene.hpp"

#include <cassert>

#include "Core/Core.hpp"

#include "Graphics/TransformUpdateReceiver.hpp"

#include "Math/Math.hpp"

#include "Memory/Allocator.hpp"

namespace kokko
{

const SceneObjectId SceneObjectId::Null = SceneObjectId{};

static bool NotNull(SceneObjectId id) { return id != SceneObjectId::Null; }

Scene::Scene(Allocator* allocator) :
	allocator(allocator),
	entityMap(allocator),
	updatedEntities(allocator),
	updatedTransforms(allocator)
{
	data = InstanceData{};
	data.count = 1; // Reserve index 0 as SceneObjectId::Null value

	this->Reallocate(512);
}

Scene::~Scene()
{
	allocator->Deallocate(data.buffer);
}

void Scene::Reallocate(unsigned int required)
{
	KOKKO_PROFILE_FUNCTION();

	if (required <= data.allocated)
		return;

	required = static_cast<unsigned int>(Math::UpperPowerOfTwo(required));

	// Reserve same amount in entity map
	entityMap.Reserve(required);

	size_t objectBytes = sizeof(Entity) + 2 * sizeof(Mat4x4f) +
		4 * sizeof(SceneObjectId) + sizeof(SceneEditTransform);

	InstanceData newData;
	newData.buffer = allocator->Allocate(required * objectBytes, "Scene.data.buffer");
	newData.count = data.count;
	newData.allocated = required;

	newData.entity = static_cast<Entity*>(newData.buffer);
	newData.local = reinterpret_cast<Mat4x4f*>(newData.entity + required);
	newData.world = newData.local + required;
	newData.parent = reinterpret_cast<SceneObjectId*>(newData.world + required);
	newData.firstChild = newData.parent + required;
	newData.nextSibling = newData.firstChild + required;
	newData.prevSibling = newData.nextSibling + required;
	newData.editTransform = reinterpret_cast<SceneEditTransform*>(newData.prevSibling + required);

	if (data.buffer != nullptr)
	{
		std::memcpy(newData.entity, data.entity, data.count * sizeof(Entity));
		std::memcpy(newData.local, data.local, data.count * sizeof(Mat4x4f));
		std::memcpy(newData.world, data.world, data.count * sizeof(Mat4x4f));
		std::memcpy(newData.parent, data.parent, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.firstChild, data.firstChild, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.nextSibling, data.nextSibling, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.prevSibling, data.prevSibling, data.count * sizeof(SceneObjectId));
		std::memcpy(newData.editTransform, data.editTransform, data.count * sizeof(SceneEditTransform));

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
		newData.editTransform[SceneObjectId::Null.i] = SceneEditTransform();
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
		mapPair->second = SceneObjectId{ id };

		data.entity[id] = e;
		data.local[id] = Mat4x4f();
		data.world[id] = Mat4x4f();
		data.parent[id] = SceneObjectId::Null;
		data.firstChild[id] = SceneObjectId::Null;
		data.nextSibling[id] = SceneObjectId::Null;
		data.prevSibling[id] = SceneObjectId::Null;
		data.editTransform[id] = SceneEditTransform();

		idsOut[i].i = id;
	}

	data.count += count;

	updatedEntities.InsertUnique(reinterpret_cast<unsigned int*>(entities), count);
}

void Scene::RemoveSceneObject(SceneObjectId id)
{
	assert(NotNull(id));

	// Remove references
	{
		// Remove from entity map
		auto* entityKv = entityMap.Lookup(data.entity[id.i].id);
		if (entityKv != nullptr)
			entityMap.Remove(entityKv);

		SceneObjectId parent = data.parent[id.i];
		SceneObjectId firstChild = data.firstChild[id.i];
		SceneObjectId prevSibling = data.prevSibling[id.i];
		SceneObjectId nextSibling = data.nextSibling[id.i];

		if (NotNull(prevSibling)) // We're not the first sibling
		{
			// nextSibling can be Null, no need to check
			data.nextSibling[prevSibling.i] = nextSibling;
		}
		else if (NotNull(parent)) // We have a parent that's not the root
		{
			// Because we didn't have prevSibling, we know we were the first child
			// nextSibling can be Null, no need to check
			data.firstChild[parent.i] = nextSibling;
		}

		if (NotNull(nextSibling)) // We have nextSibling, its prevSibling must be updated
		{
			// prevSibling can be Null, no need to check
			data.prevSibling[nextSibling.i] = prevSibling;
		}

		// Object has children, their parent must be updated
		for (SceneObjectId child = firstChild; NotNull(child); child = data.nextSibling[child.i])
			data.parent[child.i] = id;
	}

	// Swap last item in the removed object's place

	if (data.count > 2 && id.i + 1 < data.count) // We still have objects other than the root
	{
		unsigned int swapIdx = data.count - 1;

		// Update the swapped objects id in the entity map
		auto* swapKv = entityMap.Lookup(data.entity[swapIdx].id);
		if (swapKv != nullptr)
			swapKv->second = id;

		SceneObjectId swap{ swapIdx };

		SceneObjectId parent = data.parent[swapIdx];
		SceneObjectId firstChild = data.firstChild[swapIdx];
		SceneObjectId prevSibling = data.prevSibling[swapIdx];
		SceneObjectId nextSibling = data.nextSibling[swapIdx];

		// Update references pointing to the swap object

		// Swap isn't the first sibling
		if (NotNull(prevSibling))
			data.nextSibling[prevSibling.i] = id;

		// Swap has a parent that's not the root
		// Because we didn't have prevSibling, we know we were the first child
		else if (NotNull(parent))
			data.firstChild[parent.i] = id;

		// Swap has nextSibling, its prevSibling must be updated
		if (NotNull(nextSibling))
			data.prevSibling[nextSibling.i] = id;

		// Swap has children, their parent must be updated
		for (SceneObjectId child = firstChild; NotNull(child); child = data.nextSibling[child.i])
			data.parent[child.i] = id;

		// Update swap objects data to the removed objects place

		data.entity[id.i] = data.entity[swapIdx];
		data.local[id.i] = data.local[swapIdx];
		data.world[id.i] = data.world[swapIdx];
		data.parent[id.i] = parent;
		data.firstChild[id.i] = firstChild;
		data.prevSibling[id.i] = prevSibling;
		data.nextSibling[id.i] = nextSibling;
		data.editTransform[id.i] = data.editTransform[swapIdx];
	}

	--data.count;
}

void Scene::Clear()
{
	entityMap.Clear();
	data.count = 1;
}

void Scene::SetParent(SceneObjectId id, SceneObjectId parent)
{
	assert(NotNull(id));

	SceneObjectId oldParent = data.parent[id.i];

	if (oldParent != parent)
	{
		// Patch references relating to old position in hierarchy
		{
			SceneObjectId prevSibling = data.prevSibling[id.i];
			SceneObjectId nextSibling = data.nextSibling[id.i];

			// No need to check for Null values as they are valid values

			if (NotNull(prevSibling)) // We're not the first sibling
			{
				data.nextSibling[prevSibling.i] = nextSibling;
				data.prevSibling[id.i] = SceneObjectId::Null;
			}
			// Object has a parent that's not the root
			// Because object didn't have prevSibling, it must be the first child
			else if (NotNull(oldParent))
				data.firstChild[oldParent.i] = nextSibling;

			// Object has nextSibling, its prevSibling must be updated
			if (NotNull(nextSibling))
			{
				data.prevSibling[nextSibling.i] = prevSibling;
				data.nextSibling[id.i] = SceneObjectId::Null;
			}

			// No need to check for children of object
			// The SceneObjectId didn't change, so children will still be children
		}

		// Create references for new position in hierarchy

		if (NotNull(parent)) // New parent isn't root
		{
			SceneObjectId parentsChild = data.firstChild[parent.i];

			// If the new parent has a child, set this object as the prevSibling
			if (NotNull(parentsChild))
			{
				data.prevSibling[parentsChild.i] = id;
				data.nextSibling[id.i] = parentsChild;
			}

			// Set this object as the first child of the new parent
			data.firstChild[parent.i] = id;
		}

		// Finally set the new parent
		data.parent[id.i] = parent;

		UpdateWorldTransforms(id);
	}
}

void Scene::SetLocalTransform(SceneObjectId id, const Mat4x4f& transform)
{
	assert(NotNull(id));

	data.local[id.i] = transform;

	UpdateWorldTransforms(id);
}

void Scene::SetLocalAndEditTransform(SceneObjectId id, const Mat4x4f& local, const SceneEditTransform& edit)
{
	SetLocalTransform(id, local);

	data.editTransform[id.i] = edit;
}

const SceneEditTransform& Scene::GetEditTransform(SceneObjectId id)
{
	return data.editTransform[id.i];
}

void Scene::SetEditTransform(SceneObjectId id, const SceneEditTransform& editTransform)
{
	data.editTransform[id.i] = editTransform;

	Mat4x4f transform = Mat4x4f::Translate(editTransform.translation) *
		Mat4x4f::RotateEuler(editTransform.rotation) *
		Mat4x4f::Scale(editTransform.scale);

	SetLocalTransform(id, transform);
}

void Scene::UpdateWorldTransforms(SceneObjectId id)
{
	updatedEntities.InsertUnique(data.entity[id.i].id);

	// Set world transform for specified object
	if (NotNull(data.parent[id.i]))
		data.world[id.i] = data.world[data.parent[id.i].i] * data.local[id.i];
	else
		data.world[id.i] = data.local[id.i];

	SceneObjectId current = data.firstChild[id.i];
	SceneObjectId lastValid;

	while (NotNull(current))
	{
		data.world[current.i] = data.world[data.parent[current.i].i] * data.local[current.i];

		// Set the entity as updated
		updatedEntities.InsertUnique(data.entity[current.i].id);

		// Move to child
		lastValid = current;
		current = data.firstChild[current.i];

		// No children for <current>
		if (NotNull(current) == false)
		{
			// Move to next sibling
			current = data.nextSibling[lastValid.i];

			// Enter if no more siblings, find next parent's valid sibling
			// Break out when we find a valid object or hit the specified object
			while (NotNull(current) == false && lastValid.i != id.i)
			{
				lastValid = data.parent[lastValid.i];
				current = data.nextSibling[lastValid.i];
			}
		}
	}
}

void Scene::MarkUpdated(SceneObjectId id)
{
	updatedEntities.InsertUnique(data.entity[id.i].id);
}

void Scene::NotifyUpdatedTransforms(size_t receiverCount, TransformUpdateReceiver** updateReceivers)
{
	KOKKO_PROFILE_FUNCTION();

	size_t updateCount = updatedEntities.GetCount();
	updatedTransforms.Resize(updateCount);

	Entity* entities = reinterpret_cast<Entity*>(updatedEntities.GetData());

	for (size_t i = 0; i < updateCount; ++i)
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

} // namespace kokko
