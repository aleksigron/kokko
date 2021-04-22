#pragma once

#include "Entity/Entity.hpp"

class EntityManager;
class CameraSystem;
class LightManager;
class Renderer;
class MeshManager;
class Scene;

struct SceneObjectId;

class EntityView
{
private:
	EntityManager* entityManager;
	CameraSystem* cameraSystem;
	LightManager* lightManager;
	Renderer* renderer;
	MeshManager* meshManager;
	Scene* scene;

	Entity selectedEntity;

	static const size_t TextInputBufferSize = 256;
	char textInputBuffer[TextInputBufferSize];

	void DrawEntityNode(Entity entity, SceneObjectId sceneObj);
	void DrawEntityProperties();
	void DrawSceneComponent();
	void DrawCameraComponent();
	void DrawLightComponent();
	void DrawRenderComponent();

public:
	EntityView();

	void Draw(EntityManager* entityManager, CameraSystem* cameraSystem,
		LightManager* lightManager, Renderer* renderer, MeshManager* meshManager, Scene* scene);
};
