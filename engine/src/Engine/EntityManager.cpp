#include "Engine/EntityManager.hpp"

#include <cstdio>

const Entity Entity::Null = Entity(0);

EntityManager::EntityManager(Allocator* mainAllocator, Allocator* debugNameAllocator) :
	debugNameAllocator(debugNameAllocator),
	freeIndices(mainAllocator),
	debugNameMap(debugNameAllocator),
	debugNameFreelist(debugNameAllocator),
	debugNames(debugNameAllocator)
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

void EntityManager::Destroy(Entity e)
{
	freeIndices.InsertUnique(e.id);
}

void EntityManager::ClearAll()
{
	debugNameMap.Clear();

	// Set all allocated debug name strings to be free to use
	debugNameFreelist.Resize(debugNames.GetCount());
	for (unsigned int i = 0; i < debugNames.GetCount(); ++i)
		debugNameFreelist[i] = i;

	entityRangeEnd = 1;
	freeIndices.Clear();
}

const char* EntityManager::GetDebugName(Entity entity)
{
	auto* pair = debugNameMap.Lookup(entity.id);

	if (pair != nullptr)
		return debugNames[pair->second].GetCStr();
	else
		return nullptr;
}

const char* EntityManager::GetDebugNameWithFallback(Entity entity)
{
	const char* setName = GetDebugName(entity);

	if (setName != nullptr)
		return setName;
	else
	{
        
		std::snprintf(unnamedEntityBuffer, sizeof(unnamedEntityBuffer), "Entity %u", entity.id);
		return unnamedEntityBuffer;
	}
}

void EntityManager::SetDebugName(Entity entity, const char* name)
{
	auto* pair = debugNameMap.Lookup(entity.id);

	if (pair == nullptr)
	{
		uint32_t nameIndex;

		if (debugNameFreelist.GetCount() > 0)
		{
			nameIndex = debugNameFreelist.GetBack();
			debugNameFreelist.PopBack();
		}
		else
		{
			nameIndex = static_cast<uint32_t>(debugNames.GetCount());
			debugNames.PushBack(kokko::String(debugNameAllocator));
		}

		pair = debugNameMap.Insert(entity.id);
		pair->second = nameIndex;
	}

	kokko::String& nameString = debugNames.At(pair->second);
	nameString.Clear();
	nameString.Append(name);
}

void EntityManager::ClearDebugName(Entity entity)
{
	auto* pair = debugNameMap.Lookup(entity.id);

	if (pair != nullptr)
	{
		unsigned int nameIndex = pair->second;
		debugNames[nameIndex].Clear();
		debugNameFreelist.PushBack(nameIndex);

		debugNameMap.Remove(pair);
	}
}

EntityManager::Iterator::Iterator(EntityManager& entityManager) :
	manager(entityManager),
	entityIndex(1),
	freelistIndex(0)
{
	while (freelistIndex < manager.freeIndices.GetCount() &&
		entityIndex >= manager.freeIndices[freelistIndex])
	{
		++entityIndex;
		++freelistIndex;
	}
}

EntityManager::Iterator::Iterator(EntityManager& entityManager, uint32_t entityCount, uint32_t freelistCount) :
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
