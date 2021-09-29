#pragma once

#include "Core/FixedArray.hpp"
#include "Core/Pair.hpp"

#include "Engine/Entity.hpp"

#include "EditorWindow.hpp"

class World;
class MaterialManager;
class MeshManager;

struct ResourceManagers;
struct EditorContext;
struct EditorWindowInfo;

class EntityView : public EditorWindow
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

	virtual void Update(EditorContext& context) override;
};
