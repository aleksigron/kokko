#pragma once

#include "Core/FixedArray.hpp"
#include "Core/Pair.hpp"

#include "Entity/Entity.hpp"
#include "Graphics/Scene.hpp"

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

	// First one is the object that is moved, the second one is the parent
	Pair<SceneObjectId, SceneObjectId> requestSetSceneObjectParent;

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

	static const char* const SceneDragDropPayloadType;

	void DrawEntityListButtons(World* world);
	void DrawEntityNode(World* world, Entity entity, SceneObjectId sceneObj);

	static void ProcessSceneDragDropSource(SceneObjectId sceneObj, const char* entityName);
	void ProcessSceneDragDropTarget(SceneObjectId parent);

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
