#include "Entity/EntityManager.hpp"

Entity Entity::Null = Entity(0);

EntityManager::Iterator::Iterator(EntityManager& entityManager) :
	manager(entityManager),
	entityIndex(0),
	freelistIndex(0)
{
	while (freelistIndex < manager.freeIndices.GetCount() &&
		entityIndex >= manager.freeIndices[freelistIndex])
	{
		++entityIndex;
		++freelistIndex;
	}
}

EntityManager::Iterator::Iterator(EntityManager& entityManager, size_t entityCount, size_t freelistCount) :
	manager(entityManager),
	entityIndex(entityCount),
	freelistIndex(freelistCount)
{
}

EntityManager::Iterator& EntityManager::Iterator::operator++()
{
	if (entityIndex < manager.entityRangeEnd)
		++entityIndex;

	while (freelistIndex < manager.freeIndices.GetCount() &&
		entityIndex >= manager.freeIndices[freelistIndex])
	{
		++entityIndex;
		++freelistIndex;
	}

	return *this;
}

EntityManager::EntityManager(Allocator* allocator) :
	freeIndices(allocator)
{
	// Reserve index 0 as invalid value
	entityRangeEnd = 1;
}

EntityManager::~EntityManager()
{
}

Entity EntityManager::Create()
{
	unsigned int idx;
	if (freeIndices.GetCount() > 0)
	{
		idx = freeIndices[freeIndices.GetCount() - 1];
		freeIndices.PopBack();
	}
	else
	{
		idx = entityRangeEnd;
		entityRangeEnd += 1;
	}
	return Entity(idx);
}

bool EntityManager::IsAlive(Entity e) const
{
	return e.id > 0 && e.id < entityRangeEnd&& freeIndices.Contains(e.id) == false;
}

void EntityManager::Destroy(Entity e)
{
	freeIndices.InsertUnique(e.id);
}