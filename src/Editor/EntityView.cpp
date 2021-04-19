#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Entity/EntityManager.hpp"

#include "Scene/Scene.hpp"

EntityView::EntityView() :
	entityManager(nullptr),
	scene(nullptr),
	selectedEntity(Entity::Null)
{
}

void EntityView::Draw(EntityManager* entityManager, Scene* scene)
{
	this->entityManager = entityManager;
	this->scene = scene;

	ImGui::Begin("Entities");

	float fontSize = ImGui::GetFontSize();
	ImGui::BeginChild("EntityList", ImVec2(fontSize * 16.0f, 0.0f));

	for (Entity entity : *entityManager)
	{
		SceneObjectId sceneObj = scene->Lookup(entity);

		// Only draw root level objects, or entities that don't exist in the scene hierarchy
		if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
			DrawEntityNode(entity, sceneObj);
	}

	ImGui::EndChild();

	ImGui::SameLine();

	ImGui::BeginChild("EntityProps", ImVec2(0.0f, 0.0f));
	DrawEntityProperties();
	ImGui::EndChild();

	ImGui::End();
}

void EntityView::DrawEntityNode(Entity entity, SceneObjectId sceneObj)
{
	SceneObjectId firstChild = SceneObjectId::Null;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;
	
	if (sceneObj == SceneObjectId::Null)
		flags = flags | ImGuiTreeNodeFlags_Leaf;
	else
	{
		firstChild = scene->GetFirstChild(sceneObj);

		if (firstChild == SceneObjectId::Null)
			flags = flags | ImGuiTreeNodeFlags_Leaf;
	}

	if (entity == selectedEntity)
		flags = flags | ImGuiTreeNodeFlags_Selected;

	const char* entityName = entityManager->GetDebugName(entity);
	bool opened = ImGui::TreeNodeEx((void*)entity.id, flags, entityName);

	if (ImGui::IsItemClicked())
	{
		selectedEntity = entity;
	}

	if (opened)
	{
		SceneObjectId child = firstChild;

		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			DrawEntityNode(childEntity, child);

			child = scene->GetNextSibling(child);
		}

		ImGui::TreePop();
	}
}

void EntityView::DrawEntityProperties()
{
	if (selectedEntity != Entity::Null)
	{
		// Name

		const char* entityName = entityManager->GetDebugName(selectedEntity);
		std::strncpy(textInputBuffer, entityName, TextInputBufferSize);
		if (ImGui::InputText("Name", textInputBuffer, TextInputBufferSize))
		{
			if (std::strlen(textInputBuffer) > 0)
				entityManager->SetDebugName(selectedEntity, textInputBuffer);
			else
				entityManager->ClearDebugName(selectedEntity);
		}

		// Scene object

		SceneObjectId sceneObj = scene->Lookup(selectedEntity);
		if (sceneObj != SceneObjectId::Null)
		{
			if (ImGui::TreeNodeEx("Scene object"))
			{
				bool edited = false;
				SceneEditTransform transform = scene->GetEditTransform(sceneObj);

				if (ImGui::DragFloat3("Translation", transform.translation.ValuePointer(), 0.01f))
				{
					edited = true;
				}

				Vec3f rotationDegrees = Math::RadiansToDegrees(transform.rotation);
				if (ImGui::DragFloat3("Rotation", rotationDegrees.ValuePointer(), 1.0f))
				{
					transform.rotation = Math::DegreesToRadians(rotationDegrees);
					edited = true;
				}

				if (ImGui::DragFloat3("Scale", transform.scale.ValuePointer(), 0.01f))
				{
					edited = true;
				}

				if (edited)
					scene->SetEditTransform(sceneObj, transform);

				ImGui::TreePop();
			}
		}

	}
}
