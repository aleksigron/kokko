#pragma once

#include "Entity/Entity.hpp"

class Scene;
class CameraSystem;
class EntityManager;

struct SceneObjectId;

class EntityView
{
private:
	EntityManager* entityManager;
	CameraSystem* cameraSystem;
	Scene* scene;

	Entity selectedEntity;

	static const size_t TextInputBufferSize = 256;
	char textInputBuffer[TextInputBufferSize];

	void DrawEntityNode(Entity entity, SceneObjectId sceneObj);
	void DrawEntityProperties();

public:
	EntityView();

	void Draw(EntityManager* entityManager, CameraSystem* cameraSystem, Scene* scene);
};
