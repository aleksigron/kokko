#pragma once

#include "Entity/Entity.hpp"

class CameraSystem;
class Engine;
class EntityManager;
class LightManager;
class MaterialManager;
class MeshManager;
class Renderer;
class Scene;

struct SceneObjectId;

class EntityView
{
private:
	EntityManager* entityManager;

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

	void DrawEntityListButtons(Scene* scene);
	void DrawEntityNode(Scene* scene, Entity entity, SceneObjectId sceneObj);

	void DrawEntityProperties(Scene* scene);
	void DrawEntityPropertyButtons(Scene* scene);

	void DrawSceneComponent(Scene* scene);
	void DrawRenderComponent(Scene* scene);
	void DrawCameraComponent();
	void DrawLightComponent();

	void CreateEntity(Scene* scene, ComponentType* components, unsigned int componentCount);
	void DestroyEntity(Scene* scene, Entity entity);

	void AddComponent(Scene* scene, Entity entity, ComponentType componentType);
	void RemoveComponentIfExists(Scene* scene, Entity entity, ComponentType componentType);

public:
	EntityView();

	void Initialize(Engine* engine);

	void Draw(Scene* scene);
};
