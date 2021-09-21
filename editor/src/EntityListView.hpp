#pragma once

#include "Core/FixedArray.hpp"
#include "Core/Pair.hpp"

#include "Engine/Entity.hpp"

#include "Graphics/Scene.hpp"

class World;

struct SceneObjectId;
struct SelectionContext;
struct EditorWindowInfo;

class EntityListView
{
private:
	Entity requestScrollToEntity;

	// First one is the object that is moved, the second one is the parent
	Pair<SceneObjectId, SceneObjectId> requestSetSceneObjectParent;

	void DrawEntityListButtons(World* world);
	void DrawEntityNode(SelectionContext& context, World* world, Entity entity, SceneObjectId sceneObj);

	static void ProcessSceneDragDropSource(SceneObjectId sceneObj, const char* entityName);
	void ProcessSceneDragDropTarget(SceneObjectId parent);

public:
	EntityListView();

	void Draw(EditorWindowInfo& windowInfo, SelectionContext& context, World* world);
};
