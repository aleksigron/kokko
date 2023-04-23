#pragma once

#include "Core/Array.hpp"
#include "Core/FixedArray.hpp"
#include "Core/HashMap.hpp"
#include "Core/SortedArray.hpp"
#include "Core/String.hpp"
#include "Core/StringView.hpp"

#include "Engine/Entity.hpp"

class Allocator;

class EntityManager
{
private:
	Allocator* debugNameAllocator;

	uint32_t entityRangeEnd;
	kokko::SortedArray<uint32_t> freeIndices;

	// Map from entity id to index in the debug name array
	HashMap<unsigned int, unsigned int> debugNameMap;

	// Debug names array indices that are free to use
	kokko::Array<unsigned int> debugNameFreelist;

	kokko::Array<kokko::String> debugNames;

	char unnamedEntityBuffer[32];

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
		uint32_t entityIndex;
		uint32_t freelistIndex;

		friend class EntityManager;

		explicit Iterator(EntityManager& entityManager);
		Iterator(EntityManager& entityManager, uint32_t entityCount, uint32_t freelistCount);

	public:
		Iterator& operator++();

		Entity operator*() { return Entity(entityIndex); }
		Entity operator->() { return operator*(); }

		bool operator!=(const Iterator& other) { return entityIndex != other.entityIndex; }
	};

	friend class Iterator;

	Iterator begin() { return Iterator(*this); }
	Iterator end() { return Iterator(*this, entityRangeEnd, static_cast<uint32_t>(freeIndices.GetCount())); }
};
