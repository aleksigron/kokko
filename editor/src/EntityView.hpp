#pragma once

#include "Core/Optional.hpp"

#include "Engine/Entity.hpp"

#include "Resources/ResourceManagers.hpp"

#include "EditorWindow.hpp"

class World;
class MaterialManager;
class MeshManager;
class TextureManager;

struct ResourceManagers;
struct TextureId;

namespace kokko
{

class ModelManager;

namespace editor
{

struct EditorContext;
struct EditorWindowInfo;

class EntityView : public EditorWindow
{
private:
	MaterialManager* materialManager;
	MeshManager* meshManager;
	ModelManager* modelManager;
	TextureManager* textureManager;

	Entity requestDestroyEntity;

	void DrawButtons(Entity selectedEntity, World* world);

	void DrawSceneComponent(Entity selectedEntity, World* world);
	void DrawRenderComponent(EditorContext& context);
	void DrawCameraComponent(Entity selectedEntity, World* world);
	void DrawLightComponent(Entity selectedEntity, World* world);

	void DrawTerrainComponent(EditorContext& context, World* world);
	Optional<TextureId> DrawTerrainTexture(EditorContext& context, TextureId textureId, const char* label);

	void DrawParticleComponent(Entity selectedEntity, World* world);
	void DrawEnvironmentComponent(EditorContext& context, World* world);

public:
	EntityView();

	void Initialize(const ResourceManagers& resourceManagers);

	virtual void Update(EditorContext& context) override;
};

}
}
