#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Engine/World.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"

const char* const EntityView::ComponentNames[] = {
	"Scene object",
	"Update object",
	"Camera",
	"Light"
};

const char* const EntityView::SceneDragDropPayloadType = "SceneObject";

EntityView::EntityView() :
	materialManager(nullptr),
	meshManager(nullptr),
	selectedEntity(Entity::Null),
	requestScrollToEntity(Entity::Null),
	requestDestroyEntity(Entity::Null)
{
}

void EntityView::Initialize(const ResourceManagers& resourceManagers)
{
	materialManager = resourceManagers.materialManager;
	meshManager = resourceManagers.meshManager;
}

void EntityView::Draw(World* world)
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
	ImGui::PopStyleVar();

	float fontSize = ImGui::GetFontSize();

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame |
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip;

	if (ImGui::BeginTable("EntityLayout", 2, tableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		DrawEntityListButtons(world);

		if (ImGui::BeginChild("EntityList"))
		{
			EntityManager* entityManager = world->GetEntityManager();
			Scene* scene = world->GetScene();
			
			ImGuiTreeNodeFlags levelNodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth |
				ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen;

			if (ImGui::TreeNodeEx(world->GetLoadedLevelFilename().GetCStr(), levelNodeFlags))
			{
				ProcessSceneDragDropTarget(SceneObjectId::Null);

				for (Entity entity : *entityManager)
				{
					SceneObjectId sceneObj = scene->Lookup(entity);

					// Only draw root level objects, or entities that don't exist in the scene hierarchy
					if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
						DrawEntityNode(world, entity, sceneObj);
				}

				ImGui::TreePop();
			}

			ImGui::Spacing();
			ImGui::EndChild();

			if (requestSetSceneObjectParent.first != SceneObjectId::Null)
			{
				scene->SetParent(requestSetSceneObjectParent.first, requestSetSceneObjectParent.second);
				requestSetSceneObjectParent = Pair(SceneObjectId::Null, SceneObjectId::Null);
			}
		}

		ImGui::TableNextColumn();

		{
			ImGui::BeginChild("EntityProps", ImVec2(0.0f, 0.0f));
			DrawEntityProperties(world);
			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void EntityView::DrawEntityListButtons(World* world)
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * 10.0f);
	if (ImGui::BeginCombo("##CreateEntityCombo", "Create new...", ImGuiComboFlags_NoArrowButton))
	{
		if (ImGui::Selectable("Empty"))
		{
			CreateEntity(world, nullptr, 0);
		}
		if (ImGui::Selectable("Scene object"))
		{
			ComponentType component = ComponentType::Scene;
			CreateEntity(world, &component, 1);
		}
		if (ImGui::Selectable("Render object"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Render };
			CreateEntity(world, components, sizeof(components) / sizeof(components[0]));
		}
		if (ImGui::Selectable("Camera"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Camera };
			CreateEntity(world, components, sizeof(components) / sizeof(components[0]));
		}
		if (ImGui::Selectable("Light"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Light };
			CreateEntity(world, components, sizeof(components) / sizeof(components[0]));
		}

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void EntityView::DrawEntityNode(World* world, Entity entity, SceneObjectId sceneObj)
{
	EntityManager* entityManager = world->GetEntityManager();
	Scene* scene = world->GetScene();

	SceneObjectId firstChild = SceneObjectId::Null;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;

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

	if (entity == selectedEntity)
		flags = flags | ImGuiTreeNodeFlags_Selected;

	const char* entityName = entityManager->GetDebugNameWithFallback(entity);
	bool opened = ImGui::TreeNodeEx((void*)entity.id, flags, entityName);

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
		selectedEntity = entity;
	}

	if (opened)
	{
		SceneObjectId child = firstChild;

		while (child != SceneObjectId::Null)
		{
			Entity childEntity = scene->GetEntity(child);

			DrawEntityNode(world, childEntity, child);

			child = scene->GetNextSibling(child);
		}

		ImGui::TreePop();
	}
}

void EntityView::ProcessSceneDragDropSource(SceneObjectId sceneObj, const char* entityName)
{
	if (ImGui::BeginDragDropSource())
	{
		ImGui::SetDragDropPayload(SceneDragDropPayloadType, &sceneObj, sizeof(SceneObjectId));
		ImGui::Text(entityName);
		ImGui::EndDragDropSource();
	}
}

void EntityView::ProcessSceneDragDropTarget(SceneObjectId parent)
{
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(SceneDragDropPayloadType);
		if (payload != nullptr && payload->DataSize == sizeof(SceneObjectId))
		{
			SceneObjectId dragDropObj;
			std::memcpy(&dragDropObj, payload->Data, sizeof(SceneObjectId));

			requestSetSceneObjectParent = Pair(dragDropObj, parent);
		}

		ImGui::EndDragDropTarget();
	}
}

void EntityView::DrawEntityProperties(World* world)
{
	EntityManager* entityManager = world->GetEntityManager();

	if (selectedEntity != Entity::Null)
	{
		ImGui::Spacing();

		// Name

		const char* entityName = entityManager->GetDebugNameWithFallback(selectedEntity);
		std::strncpy(textInputBuffer.GetData(), entityName, textInputBuffer.GetCapacity());
		if (ImGui::InputText("Name", textInputBuffer.GetData(), textInputBuffer.GetCapacity()))
		{
			if (std::strlen(textInputBuffer.GetData()) > 0)
				entityManager->SetDebugName(selectedEntity, textInputBuffer.GetData());
			else
				entityManager->ClearDebugName(selectedEntity);
		}

		DrawEntityPropertyButtons(world);

		DrawSceneComponent(world);
		DrawRenderComponent(world);
		DrawCameraComponent(world);
		DrawLightComponent(world);

		ImGui::Spacing();
	}

	if (requestDestroyEntity != Entity::Null)
	{
		if (selectedEntity == requestDestroyEntity)
			selectedEntity = Entity::Null;

		DestroyEntity(world, requestDestroyEntity);
		requestDestroyEntity = Entity::Null;
	}
}

void EntityView::DrawEntityPropertyButtons(World* world)
{
	bool addComponent = false;
	ComponentType addComponentType;

	ImGui::PushItemWidth(ImGui::GetFontSize() * 9.0f);
	if (ImGui::BeginCombo("##AddComponentCombo", "Add component...", ImGuiComboFlags_NoArrowButton))
	{
		for (unsigned int i = 0; i < ComponentTypeCount; ++i)
		{
			if (ImGui::Selectable(ComponentNames[i], false))
			{
				addComponent = true;
				addComponentType = static_cast<ComponentType>(i);
			}
		}

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	if (addComponent)
		AddComponent(world, selectedEntity, addComponentType);

	ImGui::SameLine();

	if (ImGui::Button("Destroy entity"))
	{
		requestDestroyEntity = selectedEntity;
	}
}

void EntityView::DrawSceneComponent(World* world)
{
	Scene* scene = world->GetScene();
	SceneObjectId sceneObj = scene->Lookup(selectedEntity);

	if (sceneObj != SceneObjectId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		if (ImGui::CollapsingHeader("Scene object", &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
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
		}

		if (componentVisible == false)
			scene->RemoveSceneObject(sceneObj);
	}
}

void EntityView::DrawRenderComponent(World* world)
{
	Scene* scene = world->GetScene();
	Renderer* renderer = world->GetRenderer();

	RenderObjectId renderObj = renderer->Lookup(selectedEntity);
	if (renderObj != RenderObjectId::Null)
	{
		ImGui::Spacing();

		ImVec4 warningColor(1.0f, 0.6f, 0.0f, 1.0f);

		bool componentVisible = true;
		if (ImGui::CollapsingHeader("Render object", &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			MeshId meshId = renderer->GetMeshId(renderObj);
			const char* meshPath = nullptr;
			
			if (meshId != MeshId::Null)
			{
				meshPath = meshManager->GetPath(meshId);
			}

			// Allow editing if mesh is loaded from a file, or if there is no mesh set
			if (meshPath != nullptr || meshId == MeshId::Null)
			{
				if (meshPath == nullptr)
					meshPath = "";

				std::strncpy(textInputBuffer.GetData(), meshPath, textInputBuffer.GetCapacity());
				if (ImGui::InputText("Mesh path", textInputBuffer.GetData(), textInputBuffer.GetCapacity()))
				{
					MeshId newMeshId = meshManager->GetIdByPath(StringRef(textInputBuffer.GetData()));

					if (newMeshId != MeshId::Null)
					{
						if (newMeshId != meshId)
						{
							renderer->SetMeshId(renderObj, newMeshId);
							
							SceneObjectId sceneObj = scene->Lookup(selectedEntity);
							if (sceneObj != SceneObjectId::Null)
								scene->MarkUpdated(sceneObj);
						}
					}
					else
						ImGui::TextColored(warningColor, "Mesh not found");
				}
			}
			else // We currently don't support editing meshes that have been created directly from code
			{
				ImGui::Text("Code-generated mesh %u", meshId.i);
			}

			const char* materialPath = nullptr;
			MaterialId materialId = renderer->GetOrderData(renderObj).material;
			if (materialId != MaterialId::Null)
			{
				const MaterialData& material = materialManager->GetMaterialData(materialId);
				materialPath = material.materialPath;
			}

			// Allow editing if material is loaded from a file, or if there is no material set
			if (materialPath != nullptr || materialId == MaterialId::Null)
			{
				if (materialPath == nullptr)
					materialPath = "";

				std::strncpy(textInputBuffer.GetData(), materialPath, textInputBuffer.GetCapacity());
				if (ImGui::InputText("Material path", textInputBuffer.GetData(), textInputBuffer.GetCapacity()))
				{
					MaterialId newMatId = materialManager->GetIdByPath(StringRef(textInputBuffer.GetData()));

					if (newMatId != MaterialId::Null)
					{
						if (newMatId != materialId)
						{
							const MaterialData& material = materialManager->GetMaterialData(newMatId);

							RenderOrderData order;
							order.material = newMatId;
							order.transparency = material.transparency;

							renderer->SetOrderData(renderObj, order);
						}
					}
					else
						ImGui::TextColored(warningColor, "Material not found");
				}
			}
			else // We currently don't support editing materials that have created as a copy from another material
			{
				ImGui::Text("Copied material %u", materialId.i);
			}

			if (componentVisible == false)
				renderer->RemoveRenderObject(renderObj);
		}
	}
}

void EntityView::DrawCameraComponent(World* world)
{
	CameraSystem* cameraSystem = world->GetCameraSystem();
	CameraId cameraId = cameraSystem->Lookup(selectedEntity);

	if (cameraId != CameraId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		if (ImGui::CollapsingHeader("Camera", &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool edited = false;
			ProjectionParameters params = cameraSystem->GetData(cameraId);

			if (ImGui::BeginCombo("Projection", CameraSystem::GetProjectionTypeDisplayName(params.projection)))
			{
				for (int i = 0; i < 2; ++i)
				{
					ProjectionType typeItr = static_cast<ProjectionType>(i);
					bool isSelected = (params.projection == typeItr);
					if (ImGui::Selectable(CameraSystem::GetProjectionTypeDisplayName(typeItr), &isSelected))
					{
						params.projection = static_cast<ProjectionType>(i);
						edited = true;
					}
				}

				ImGui::EndCombo();
			}

			if (params.projection == ProjectionType::Perspective)
			{
				float maxFovDegrees = std::round(160.0f / params.aspect);
				float fovDegrees = Math::RadiansToDegrees(params.perspectiveFieldOfView);
				if (ImGui::DragFloat("Field of view", &fovDegrees, 0.1f, 0.1f, maxFovDegrees))
				{
					params.perspectiveFieldOfView = Math::DegreesToRadians(fovDegrees);
					edited = true;
				}

				if (ImGui::DragFloat("Near distance", &params.perspectiveNear, 0.001f, 0.001f, 0.0f))
					edited = true;

				if (ImGui::DragFloat("Far distance", &params.perspectiveFar, 0.1f, 0.1f, 0.0f))
					edited = true;
			}
			else
			{
				if (ImGui::DragFloat("Height", &params.orthographicHeight, 0.01f, 0.0f, 0.0f))
					edited = true;

				if (ImGui::DragFloat("Near distance", &params.orthographicNear, 0.01f, 0.0f, 0.0f))
					edited = true;

				if (ImGui::DragFloat("Far distance", &params.orthographicFar, 0.01f, 0.0f, 0.0f))
					edited = true;
			}

			if (edited)
				cameraSystem->SetData(cameraId, params);

			if (componentVisible == false)
				cameraSystem->RemoveComponent(cameraId);
		}
	}
}

void EntityView::DrawLightComponent(World* world)
{
	LightManager* lightManager = world->GetLightManager();
	LightId lightId = lightManager->Lookup(selectedEntity);

	if (lightId != LightId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		if (ImGui::CollapsingHeader("Light", &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool edited = false;
			LightType lightType = lightManager->GetLightType(lightId);

			if (ImGui::BeginCombo("Type", LightManager::GetLightTypeDisplayName(lightType)))
			{
				for (int i = 0; i < 3; ++i)
				{
					LightType typeItr = static_cast<LightType>(i);
					bool isSelected = (lightType == typeItr);
					if (ImGui::Selectable(LightManager::GetLightTypeDisplayName(typeItr), &isSelected))
					{
						lightType = static_cast<LightType>(i);
						lightManager->SetLightType(lightId, lightType);
					}
				}

				ImGui::EndCombo();
			}

			Vec3f lightColor = lightManager->GetColor(lightId);
			ImGuiColorEditFlags colorEditFlags = ImGuiColorEditFlags_NoAlpha |
				ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR;
			if (ImGui::ColorEdit3("Color", lightColor.ValuePointer(), colorEditFlags))
				lightManager->SetColor(lightId, lightColor);

			if (lightType != LightType::Directional)
			{
				float radius = lightManager->GetRadius(lightId);
				if (ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f))
					lightManager->SetRadius(lightId, radius);

				if (ImGui::Button("Auto-calculate radius"))
					lightManager->SetRadiusFromColor(lightId);
			}

			if (lightType == LightType::Spot)
			{
				float spotAngle = lightManager->GetSpotAngle(lightId);
				float angleDegrees = Math::RadiansToDegrees(spotAngle);
				if (ImGui::DragFloat("Spot angle", &angleDegrees, 0.1f, 0.1f, 180.0f))
				{
					spotAngle = Math::DegreesToRadians(angleDegrees);
					lightManager->SetSpotAngle(lightId, spotAngle);
				}
			}

			// Currently shadows are only supported on directional lights
			if (lightType == LightType::Directional)
			{
				bool shadowCasting = lightManager->GetShadowCasting(lightId);
				if (ImGui::Checkbox("Cast shadows", &shadowCasting))
					lightManager->SetShadowCasting(lightId, shadowCasting);
			}

			if (componentVisible == false)
				lightManager->RemoveLight(lightId);
		}
	}
}

void EntityView::CreateEntity(World* world, ComponentType* components, unsigned int componentCount)
{
	EntityManager* entityManager = world->GetEntityManager();

	Entity entity = entityManager->Create();

	for (unsigned int i = 0; i < componentCount; ++i)
		AddComponent(world, entity, components[i]);

	selectedEntity = entity;
	requestScrollToEntity = entity;
}

void EntityView::DestroyEntity(World* world, Entity entity)
{
	EntityManager* entityManager = world->GetEntityManager();

	for (size_t i = 0; i < ComponentTypeCount; ++i)
		RemoveComponentIfExists(world, entity, static_cast<ComponentType>(i));

	entityManager->Destroy(entity);
}

void EntityView::AddComponent(World* world, Entity entity, ComponentType componentType)
{
	Scene* scene = world->GetScene();
	Renderer* renderer = world->GetRenderer();
	CameraSystem* cameraSystem = world->GetCameraSystem();
	LightManager* lightManager = world->GetLightManager();

	switch (componentType)
	{
	case EntityView::ComponentType::Scene:
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj == SceneObjectId::Null)
			scene->AddSceneObject(entity);
		break;
	}
	case EntityView::ComponentType::Render:
	{
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj == RenderObjectId::Null)
			renderer->AddRenderObject(entity);
		break;
	}
	case EntityView::ComponentType::Camera:
	{
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId == CameraId::Null)
			cameraSystem->AddComponentToEntity(entity);
		break;
	}
	case EntityView::ComponentType::Light:
	{
		LightId lightId = lightManager->Lookup(entity);
		if (lightId == LightId::Null)
			lightManager->AddLight(entity);
		break;
	}
	default:
		break;
	}
}

void EntityView::RemoveComponentIfExists(World* world, Entity entity, ComponentType componentType)
{
	Scene* scene = world->GetScene();
	Renderer* renderer = world->GetRenderer();
	CameraSystem* cameraSystem = world->GetCameraSystem();
	LightManager* lightManager = world->GetLightManager();

	switch (componentType)
	{
	case EntityView::ComponentType::Scene:
	{
		SceneObjectId sceneObj = scene->Lookup(entity);
		if (sceneObj != SceneObjectId::Null)
			scene->RemoveSceneObject(sceneObj);
		break;
	}
	case EntityView::ComponentType::Render:
	{
		RenderObjectId renderObj = renderer->Lookup(entity);
		if (renderObj != RenderObjectId::Null)
			renderer->RemoveRenderObject(renderObj);
		break;
	}
	case EntityView::ComponentType::Camera:
	{
		CameraId cameraId = cameraSystem->Lookup(entity);
		if (cameraId != CameraId::Null)
			cameraSystem->RemoveComponent(cameraId);
		break;
	}
	case EntityView::ComponentType::Light:
	{
		LightId lightId = lightManager->Lookup(entity);
		if (lightId != LightId::Null)
			lightManager->RemoveLight(lightId);
		break;
	}
	default:
		break;
	}
}
