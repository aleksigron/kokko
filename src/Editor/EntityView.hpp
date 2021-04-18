#pragma once

#include "Entity/Entity.hpp"

class Scene;
class EntityManager;

struct SceneObjectId;

class EntityView
{
private:
	Entity selectedEntity;

	void DrawEntityNode(Scene* scene, Entity entity, SceneObjectId sceneObj);

public:
	EntityView();

	void Draw(EntityManager* entityManager, Scene* scene);
};
