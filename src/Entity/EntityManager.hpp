#pragma once

#include "Core/Array.hpp"
#include "Core/Queue.hpp"
#include "Core/SortedArray.hpp"

#include "Entity/Entity.hpp"

#include "Memory/Allocator.hpp"

class EntityManager
{
public:
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

private:
	uint32_t entityRangeEnd;
	SortedArray<uint32_t> freeIndices;

	friend class Iterator;

public:
	EntityManager(Allocator* allocator);
	~EntityManager();

	Entity Create();
	bool IsAlive(Entity e) const;
	void Destroy(Entity e);

	Iterator begin() { return Iterator(*this); }
	Iterator end() { return Iterator(*this, entityRangeEnd, freeIndices.GetCount()); }
};
