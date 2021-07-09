#pragma once

#include "Core/Array.hpp"
#include "Core/HashMap.hpp"

#include "Engine/Entity.hpp"

template <typename ComponentData, typename ComponentId>
class ComponentSystemDefaultImpl
{
public:
	struct EntityComponent
	{
		ComponentData data;
		Entity entity;
	};

	ComponentSystemDefaultImpl(Allocator* allocator) :
		componentData(allocator),
		entityMap(allocator)
	{
		// Reserve index 0 as the null component ID
		EntityComponent& component = componentData.PushBack();
		component.entity = Entity::Null;
		component.data = ComponentData();
	}

	ComponentSystemDefaultImpl(const ComponentSystemDefaultImpl& other) = delete;
	ComponentSystemDefaultImpl(ComponentSystemDefaultImpl&& other) = delete;

	~ComponentSystemDefaultImpl() = default;
	// No virtual destructor, because this class interface shouldn't be used directly

	ComponentSystemDefaultImpl& operator=(const ComponentSystemDefaultImpl& other) = delete;
	ComponentSystemDefaultImpl& operator=(ComponentSystemDefaultImpl&& other) = delete;

	ComponentId Lookup(Entity e)
	{
		HashMap<unsigned int, ComponentId>::KeyValuePair* pair = entityMap.Lookup(e.id);
		return pair != nullptr ? pair->second : ComponentId::Null;
	}

	ComponentId AddComponentToEntity(Entity e)
	{
		unsigned int id = componentData.GetCount();

		auto mapPair = entityMap.Insert(e.id);
		mapPair->second.i = id;

		EntityComponent& component = componentData.PushBack();
		component.entity = e;
		component.data = ComponentData();
		
		return ComponentId{ id };
	}

	void RemoveComponent(ComponentId id)
	{
		Entity entity = componentData[id.i].entity;

		auto* pair = entityMap.Lookup(entity.id);
		if (pair != nullptr)
			entityMap.Remove(pair);

		// We could support deinit operations here, but for now we don't need them

		if (id.i + 1 < componentData.GetCount()) // Need to swap
		{
			const EntityComponent& swap = componentData.GetBack();

			auto* swapKv = entityMap.Lookup(swap.entity.id);
			if (swapKv != nullptr)
				swapKv->second = id;

			componentData[id.i] = swap;
		}

		componentData.PopBack();
	}

	void RemoveAll()
	{
		entityMap.Clear();
		componentData.Resize(1);
	}

	const ComponentData& GetData(ComponentId id) const
	{
		return componentData[id.i].data;
	}

	void SetData(ComponentId id, const ComponentData& data)
	{
		componentData[id.i].data = data;
	}

	typename Array<EntityComponent>::Iterator begin() { return ++componentData.begin(); }
	typename Array<EntityComponent>::Iterator end() { return componentData.end(); }

private:
	Array<EntityComponent> componentData;
	HashMap<unsigned int, ComponentId> entityMap;
};
