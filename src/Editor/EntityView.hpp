#pragma once

#include "Core/FixedArray.hpp"

#include "Entity/Entity.hpp"

class World;
class MaterialManager;
class MeshManager;

struct ResourceManagers;
struct SceneObjectId;

class EntityView
{
private:
	MaterialManager* materialManager;
	MeshManager* meshManager;

	Entity selectedEntity;
	Entity requestScrollToEntity;
	Entity requestDestroyEntity;

	FixedArray<char, 256> textInputBuffer;

	// TODO: Make reusable system to do operations on components
	enum class ComponentType
	{
		Scene,
		Render,
		Camera,
		Light
	};
	static const size_t ComponentTypeCount = 4;
	static const char* const ComponentNames[ComponentTypeCount];

	void DrawEntityListButtons(World* world);
	void DrawEntityNode(World* world, Entity entity, SceneObjectId sceneObj);

	void DrawEntityProperties(World* world);
	void DrawEntityPropertyButtons(World* world);

	void DrawSceneComponent(World* world);
	void DrawRenderComponent(World* world);
	void DrawCameraComponent(World* world);
	void DrawLightComponent(World* world);

	void CreateEntity(World* world, ComponentType* components, unsigned int componentCount);
	void DestroyEntity(World* world, Entity entity);

	void AddComponent(World* world, Entity entity, ComponentType componentType);
	void RemoveComponentIfExists(World* world, Entity entity, ComponentType componentType);

public:
	EntityView();

	void Initialize(const ResourceManagers& resourceManagers);

	void Draw(World* world);
};
