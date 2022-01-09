#pragma once

#include "Engine/Entity.hpp"

#include "EditorWindow.hpp"

class World;
class MaterialManager;
class MeshManager;

struct ResourceManagers;

namespace kokko
{
namespace editor
{

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
	void DrawRenderComponent(EditorContext& context);
	void DrawCameraComponent(Entity selectedEntity, World* world);
	void DrawLightComponent(Entity selectedEntity, World* world);
	void DrawTerrainComponent(Entity selectedEntity, World* world);
	void DrawParticleComponent(Entity selectedEntity, World* world);
	void DrawEnvironmentComponent(EditorContext& context, World* world);

public:
	EntityView();

	void Initialize(MaterialManager* materialManager, MeshManager* meshManager);

	virtual void Update(EditorContext& context) override;
};

}
}
