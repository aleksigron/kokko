#include "Editor/EditorCore.hpp"

#include "Core/Core.hpp"

#include "Engine/World.hpp"

#include "Resources/LevelSerializer.hpp"

EditorCore::EditorCore(Allocator* allocator) :
	copiedEntity(allocator),
	selectionContext{}
{
}

void EditorCore::CopyEntity(World* world)
{
	if (selectionContext.selectedEntity != Entity::Null)
	{
		LevelSerializer* serializer = world->GetSerializer();

		ArrayView<Entity> entities = ArrayView(&selectionContext.selectedEntity, 1);
		serializer->SerializeEntitiesToString(entities, copiedEntity);
		
		KK_LOG_DEBUG("Copied entity:\n{}", copiedEntity.GetCStr());
	}
}

void EditorCore::PasteEntity(World* world)
{
	if (copiedEntity.GetLength() > 0)
	{
		KK_LOG_DEBUG("Pasting entity:\n{}", copiedEntity.GetCStr());

		LevelSerializer* serializer = world->GetSerializer();

		SceneObjectId parent = SceneObjectId::Null;

		serializer->DeserializeEntitiesFromString(copiedEntity.GetCStr(), parent);
	}
}
