#pragma once

#include "Entity/Entity.hpp"

class CameraSystem;
class Engine;
class EntityManager;
class LightManager;
class MaterialManager;
class MeshManager;
class Renderer;
class World;

struct SceneObjectId;

class EntityView
{
private:
	EntityManager* entityManager;
	World* world;

	// Component systems
	Renderer* renderer;
	LightManager* lightManager;
	CameraSystem* cameraSystem;

	// Resource managers
	MaterialManager* materialManager;
	MeshManager* meshManager;

	Entity selectedEntity;
	Entity requestScrollToEntity;
	Entity requestDestroyEntity;

	static const size_t TextInputBufferSize = 256;
	char textInputBuffer[TextInputBufferSize];

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

	void DrawEntityListButtons();
	void DrawEntityNode(Entity entity, SceneObjectId sceneObj);

	void DrawEntityProperties();
	void DrawEntityPropertyButtons();

	void DrawSceneComponent();
	void DrawRenderComponent();
	void DrawCameraComponent();
	void DrawLightComponent();

	void CreateEntity(ComponentType* components, unsigned int componentCount);
	void DestroyEntity(Entity entity);

	void AddComponent(Entity entity, ComponentType componentType);
	void RemoveComponentIfExists(Entity entity, ComponentType componentType);

public:
	EntityView();

	void Initialize(Engine* engine);

	void Draw();
};
