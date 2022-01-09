#pragma once

#include "Core/ArrayView.hpp"

class World;
struct Entity;

namespace kokko
{

enum class EntityComponentType
{
	Scene,
	Render,
	Camera,
	Light,
	Terrain,
	Particle,
	Environment
};

class EntityFactory
{
public:
	static const size_t ComponentTypeCount = 7;

	static Entity CreateEntity(World* world, ArrayView<EntityComponentType> components);
	static void DestroyEntity(World* world, Entity entity);

	static void AddComponent(World* world, Entity entity, EntityComponentType component);
	static void RemoveComponentIfExists(World* world, Entity entity, EntityComponentType component);

	static const char* GetComponentTypeName(size_t typeIndex);
	static const char* GetComponentTypeName(EntityComponentType component);

private:
	static const char* const ComponentNames[ComponentTypeCount];
};

}
