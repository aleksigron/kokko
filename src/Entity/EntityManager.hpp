#pragma once

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"

#include "Entity/Entity.hpp"

class Allocator;
struct StringRef;

class EntityManager
{
private:
	Allocator* debugNameAllocator;

	uint32_t entityRangeEnd;
	SortedArray<uint32_t> freeIndices;

	// Map from entity id to index in the debug name array
	HashMap<unsigned int, unsigned int> debugNameMap;

	// Debug names array indices that are free to use
	Array<unsigned int> debugNameFreelist;

	Array<String> debugNames;

	static const size_t UnnamedEntityBufferLength = 32;
	char unnamedEntityBuffer[UnnamedEntityBufferLength];

public:
	EntityManager(Allocator* mainAllocator, Allocator* debugNameAllocator);
	~EntityManager();

	Entity Create();
	void Destroy(Entity e);

	void ClearAll();

	/*
	* Will return nullptr if no debug name is set for the entity.
	*/
	const char* GetDebugName(Entity entity);

	/*
	* Will never return nullptr. If no debug name is set, a fallback
	* name is generated, such as "Entity 1". The returned name is only valid
	* until the next call to GetDebugNameWithFallback, SetDebugName or ClearDebugName.
	*/
	const char* GetDebugNameWithFallback(Entity entity);

	void SetDebugName(Entity entity, const char* name);
	void ClearDebugName(Entity entity);

	class Iterator
	{
	private:
		EntityManager& manager;
		size_t entityIndex;
		size_t freelistIndex;

		friend class EntityManager;

		explicit Iterator(EntityManager& entityManager);
		Iterator(EntityManager& entityManager, size_t entityCount, size_t freelistCount);

	public:
		Iterator& operator++();

		Entity operator*() { return Entity(entityIndex); }
		Entity operator->() { return operator*(); }

		bool operator!=(const Iterator& other) { return entityIndex != other.entityIndex; }
	};

	friend class Iterator;

	Iterator begin() { return Iterator(*this); }
	Iterator end() { return Iterator(*this, entityRangeEnd, freeIndices.GetCount()); }
};
