#include "EntityListView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Engine/EntityFactory.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Resources/AssetLibrary.hpp"

#include "App/EditorConstants.hpp"
#include "App/EditorContext.hpp"

namespace kokko
{
namespace editor
{

EntityListView::EntityListView() :
	EditorWindow("Entities", EditorWindowGroup::Regular),
	requestScrollToEntity(Entity::Null),
	requestSetSceneObjectParent(SceneObjectId::Null, SceneObjectId::Null)
{
}

void EntityListView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
		if (ImGui::Begin(windowTitle, &windowIsOpen, windowFlags))
		{
			World* world = context.world;
			DrawEntityListButtons(world);

			if (ImGui::BeginChild("EntityList"))
			{
				EntityManager* entityManager = world->GetEntityManager();
				Scene* scene = world->GetScene();

				context.temporaryString.Assign("Unnamed level");
				if (context.loadedLevel.HasValue())
				{
					auto levelAsset = context.assetLibrary->FindAssetByUid(context.loadedLevel.GetValue());
					if (levelAsset != nullptr)
					{
						context.temporaryString.Assign(levelAsset->GetFilename());
					}
				}

				ImGuiTreeNodeFlags levelNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
					ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen;

				if (ImGui::TreeNodeEx(context.temporaryString.GetCStr(), levelNodeFlags))
				{
					ProcessSceneDragDropTarget(SceneObjectId::Null);

					for (Entity entity : *entityManager)
					{
						SceneObjectId sceneObj = scene->Lookup(entity);

						// Only draw root level objects, or entities that don't exist in the scene hierarchy
						if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
							DrawEntityNode(context, world, entity, sceneObj);
					}

					// No TreePop() because we're using CollapsingHeader, and therefore NoTreePushOnOpen
				}

				ImGui::Spacing();

				if (requestSetSceneObjectParent.first != SceneObjectId::Null)
				{
					scene->SetParent(requestSetSceneObjectParent.first, requestSetSceneObjectParent.second);
					requestSetSceneObjectParent = Pair(SceneObjectId::Null, SceneObjectId::Null);
				}
			}

			ImGui::EndChild();
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
		ImGui::PopStyleVar(); // ImGuiStyleVar_WindowPadding
	}
}

void EntityListView::DrawEntityListButtons(World* world)
{
	const float spacingH = std::floor(ImGui::GetFontSize() * 0.4f);

	ImGui::Spacing();
	ImGui::Indent(spacingH);
	float availWidth = ImGui::GetContentRegionAvail().x;
	ImGui::SetNextItemWidth(availWidth - spacingH);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));
	if (ImGui::BeginCombo("##CreateEntityCombo", "Create entity...", ImGuiComboFlags_NoArrowButton))
	{
		if (ImGui::Selectable("Empty"))
		{
			EntityFactory::CreateEntity(world, ArrayView<EntityComponentType>());
		}
		if (ImGui::Selectable("Scene object"))
		{
			EntityComponentType component = EntityComponentType::Scene;
			EntityFactory::CreateEntity(world, ArrayView(&component, 1));
		}
		if (ImGui::Selectable("Mesh object"))
		{
			EntityComponentType components[] = { EntityComponentType::Scene, EntityComponentType::Mesh };
			EntityFactory::CreateEntity(world, ArrayView(components));
		}
		if (ImGui::Selectable("Camera"))
		{
			EntityComponentType components[] = { EntityComponentType::Scene, EntityComponentType::Camera };
			EntityFactory::CreateEntity(world, ArrayView(components));
		}
		if (ImGui::Selectable("Light"))
		{
			EntityComponentType components[] = { EntityComponentType::Scene, EntityComponentType::Light };
			EntityFactory::CreateEntity(world, ArrayView(components));
		}

		ImGui::EndCombo();
	}
	ImGui::PopStyleVar(); // ImGuiStyleVar_WindowPadding
	ImGui::Unindent(spacingH);
	ImGui::Separator();
	ImGui::Spacing();
}

void EntityListView::DrawEntityNode(EditorContext& context, World* world, Entity entity, SceneObjectId sceneObj)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	SceneObjectId firstChild = SceneObjectId::Null;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap;

	if (sceneObj != SceneObjectId::Null)
	{
		firstChild = scene->GetFirstChild(sceneObj);

		if (firstChild == SceneObjectId::Null)
			flags = flags | ImGuiTreeNodeFlags_Leaf;
	}
	else
	{
		flags = flags | ImGuiTreeNodeFlags_Leaf;
	}

	if (entity == context.selectedEntity)
		flags = flags | ImGuiTreeNodeFlags_Selected;

	const char* entityName = entityManager->GetDebugNameWithFallback(entity);
	void* nodeId = reinterpret_cast<void*>(static_cast<size_t>(entity.id));
	bool opened = ImGui::TreeNodeEx(nodeId, flags, "%s", entityName);

	if (sceneObj != SceneObjectId::Null)
	{
		ProcessSceneDragDropSource(sceneObj, entityName);
		ProcessSceneDragDropTarget(sceneObj);
	}

	if (entity == requestScrollToEntity)
	{
		ImGui::SetScrollHereY();
		requestScrollToEntity = Entity::Null;
	}

	if (ImGui::IsItemClicked())
	{
		context.selectedEntity = entity;
	}

	if (opened)
	{
		SceneObjectId child = firstChild;

		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			DrawEntityNode(context, world, childEntity, child);

			child = scene->GetNextSibling(child);
		}

		ImGui::TreePop();
	}
}

void EntityListView::ProcessSceneDragDropSource(SceneObjectId sceneObj, const char* entityName)
{
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload(EditorConstants::SceneDragDropType, &sceneObj, sizeof(SceneObjectId));
		ImGui::Text("%s", entityName);
		ImGui::EndDragDropSource();
	}
}

void EntityListView::ProcessSceneDragDropTarget(SceneObjectId parent)
{
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConstants::SceneDragDropType);
		if (payload != nullptr && payload->DataSize == sizeof(SceneObjectId))
		{
			SceneObjectId dragDropObj;
			std::memcpy(&dragDropObj, payload->Data, sizeof(SceneObjectId));

			requestSetSceneObjectParent = Pair(dragDropObj, parent);
		}

		ImGui::EndDragDropTarget();
	}
}

}
}
