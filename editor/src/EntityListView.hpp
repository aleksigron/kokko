#pragma once

#include "Core/FixedArray.hpp"
#include "Core/Pair.hpp"

#include "Engine/Entity.hpp"

#include "Graphics/Scene.hpp"

#include "EditorWindow.hpp"

class World;

struct SceneObjectId;
struct EditorContext;
struct EditorWindowInfo;

class EntityListView : public EditorWindow
{
private:
	Entity requestScrollToEntity;

	// First one is the object that is moved, the second one is the parent
	Pair<SceneObjectId, SceneObjectId> requestSetSceneObjectParent;

	void DrawEntityListButtons(World* world);
	void DrawEntityNode(EditorContext& context, World* world, Entity entity, SceneObjectId sceneObj);

	static void ProcessSceneDragDropSource(SceneObjectId sceneObj, const char* entityName);
	void ProcessSceneDragDropTarget(SceneObjectId parent);

public:
	EntityListView();

	virtual void Update(EditorContext& context) override;
};
