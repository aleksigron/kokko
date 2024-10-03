#pragma once

#include "Core/Optional.hpp"

#include "Engine/Entity.hpp"

#include "Resources/ResourceManagers.hpp"

#include "EditorWindow.hpp"

namespace kokko
{

class MaterialManager;
class MeshManager;
class ModelManager;
class TextureManager;
class World;

struct ResourceManagers;
struct TextureId;

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
	void DrawMeshComponent(EditorContext& context);
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

} // namespace editor
} // namespace kokko
