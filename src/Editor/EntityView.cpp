#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Entity/EntityManager.hpp"

EntityView::EntityView()
{

}

void EntityView::Draw(EntityManager* entityManager)
{
	ImGui::Begin("Entities");
	for (Entity entity : *entityManager)
	{
		ImGui::Text("Entity %u", entity.id);
	}
	ImGui::End();
}
