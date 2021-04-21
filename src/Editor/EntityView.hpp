#pragma once

#include "Entity/Entity.hpp"

class EntityManager;
class CameraSystem;
class LightManager;
class Scene;

struct SceneObjectId;

class EntityView
{
private:
	EntityManager* entityManager;
	CameraSystem* cameraSystem;
	LightManager* lightManager;
	Scene* scene;

	Entity selectedEntity;

	static const size_t TextInputBufferSize = 256;
	char textInputBuffer[TextInputBufferSize];

	void DrawEntityNode(Entity entity, SceneObjectId sceneObj);
	void DrawEntityProperties();
	void DrawSceneComponent();
	void DrawCameraComponent();
	void DrawLightComponent();

public:
	EntityView();

	void Draw(EntityManager* entityManager, CameraSystem* cameraSystem,
		LightManager* lightManager, Scene* scene);
};
