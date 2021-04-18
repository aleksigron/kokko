#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Entity/EntityManager.hpp"

EntityView::EntityView() :
	selectedEntity(Entity::Null)
{

}

void EntityView::Draw(EntityManager* entityManager)
{
	ImGui::Begin("Entities");

	for (Entity entity : *entityManager)
	{
		// TODO: Check that this entity doesn't have a parent in the hierarchy
		DrawEntityNode(entity);
	}

	ImGui::End();
}

void EntityView::DrawEntityNode(Entity entity)
{
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow;

	if (entity == selectedEntity)
		flags = flags | ImGuiTreeNodeFlags_Selected;

	bool opened = ImGui::TreeNodeEx((void*)entity.id, flags, "Entity %u", entity.id);

	if (ImGui::IsItemClicked())
	{
		selectedEntity = entity;
	}

	if (opened)
	{
		// TODO: Draw children
		ImGui::TreePop();
	}
}
