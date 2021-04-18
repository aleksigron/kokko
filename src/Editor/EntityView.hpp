#pragma once

#include "Entity/Entity.hpp"

class EntityManager;

class EntityView
{
private:
	Entity selectedEntity;

	void DrawEntityNode(Entity entity);

public:
	EntityView();

	void Draw(EntityManager* entityManager);
};
