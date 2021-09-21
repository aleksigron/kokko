#pragma once

#include "Core/FixedArray.hpp"
#include "Core/Pair.hpp"

#include "Engine/Entity.hpp"

class World;
class MaterialManager;
class MeshManager;

struct ResourceManagers;
struct SelectionContext;
struct EditorWindowInfo;

class EntityView
{
private:
	MaterialManager* materialManager;
	MeshManager* meshManager;

	Entity requestDestroyEntity;

	void DrawButtons(Entity selectedEntity, World* world);

	void DrawSceneComponent(Entity selectedEntity, World* world);
	void DrawRenderComponent(Entity selectedEntity, World* world);
	void DrawCameraComponent(Entity selectedEntity, World* world);
	void DrawLightComponent(Entity selectedEntity, World* world);
	void DrawTerrainComponent(Entity selectedEntity, World* world);
	void DrawParticleComponent(Entity selectedEntity, World* world);

public:
	EntityView();

	void Initialize(MaterialManager* materialManager, MeshManager* meshManager);

	void Draw(EditorWindowInfo& windowInfo, SelectionContext& context, World* world);
};
