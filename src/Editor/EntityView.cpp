#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Graphics/World.hpp"

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

EntityView::EntityView() :
	entityManager(nullptr),
	world(nullptr),
	renderer(nullptr),
	lightManager(nullptr),
	cameraSystem(nullptr),
	materialManager(nullptr),
	meshManager(nullptr),
	selectedEntity(Entity::Null),
	requestScrollToEntity(Entity::Null),
	requestDestroyEntity(Entity::Null)
{
	std::memset(textInputBuffer, 0, TextInputBufferSize);
}

void EntityView::Initialize(Engine* engine)
{
	entityManager = engine->GetEntityManager();
	world = engine->GetWorld();
	renderer = engine->GetRenderer();
	lightManager = engine->GetLightManager();
	cameraSystem = engine->GetCameraSystem();
	materialManager = engine->GetMaterialManager();
	meshManager = engine->GetMeshManager();
}

void EntityView::Draw()
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

		DrawEntityListButtons();

		if (ImGui::BeginChild("EntityList"))
		{
			for (Entity entity : *entityManager)
			{
				SceneObjectId sceneObj = world->Lookup(entity);

				// Only draw root level objects, or entities that don't exist in the scene hierarchy
				if (sceneObj == SceneObjectId::Null || world->GetParent(sceneObj) == SceneObjectId::Null)
					DrawEntityNode(entity, sceneObj);
			}
			ImGui::Spacing();
			ImGui::EndChild();
		}

		ImGui::TableNextColumn();

		{
			ImGui::BeginChild("EntityProps", ImVec2(0.0f, 0.0f));
			DrawEntityProperties();
			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void EntityView::DrawEntityListButtons()
{
	ImGui::PushItemWidth(ImGui::GetFontSize() * 10.0f);
	if (ImGui::BeginCombo("##CreateEntityCombo", "Create new...", ImGuiComboFlags_NoArrowButton))
	{
		if (ImGui::Selectable("Empty"))
		{
			CreateEntity(nullptr, 0);
		}
		if (ImGui::Selectable("Scene object"))
		{
			ComponentType component = ComponentType::Scene;
			CreateEntity(&component, 1);
		}
		if (ImGui::Selectable("Render object"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Render };
			CreateEntity(components, sizeof(components) / sizeof(components[0]));
		}
		if (ImGui::Selectable("Camera"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Camera };
			CreateEntity(components, sizeof(components) / sizeof(components[0]));
		}
		if (ImGui::Selectable("Light"))
		{
			ComponentType components[] = { ComponentType::Scene, ComponentType::Light };
			CreateEntity(components, sizeof(components) / sizeof(components[0]));
		}

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
}

void EntityView::DrawEntityNode(Entity entity, SceneObjectId sceneObj)
{
	SceneObjectId firstChild = SceneObjectId::Null;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	
	if (sceneObj == SceneObjectId::Null)
		flags = flags | ImGuiTreeNodeFlags_Leaf;
	else
	{
		firstChild = world->GetFirstChild(sceneObj);

		if (firstChild == SceneObjectId::Null)
			flags = flags | ImGuiTreeNodeFlags_Leaf;
	}

	if (entity == selectedEntity)
		flags = flags | ImGuiTreeNodeFlags_Selected;

	const char* entityName = entityManager->GetDebugNameWithFallback(entity);
	bool opened = ImGui::TreeNodeEx((void*)entity.id, flags, entityName);

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
			Entity childEntity = world->GetEntity(child);

			DrawEntityNode(childEntity, child);

			child = world->GetNextSibling(child);
		}

		ImGui::TreePop();
	}
}

void EntityView::DrawEntityProperties()
{
	if (selectedEntity != Entity::Null)
	{
		ImGui::Spacing();

		// Name

		const char* entityName = entityManager->GetDebugNameWithFallback(selectedEntity);
		std::strncpy(textInputBuffer, entityName, TextInputBufferSize);
		if (ImGui::InputText("Name", textInputBuffer, TextInputBufferSize))
		{
			if (std::strlen(textInputBuffer) > 0)
				entityManager->SetDebugName(selectedEntity, textInputBuffer);
			else
				entityManager->ClearDebugName(selectedEntity);
		}

		DrawEntityPropertyButtons();

		DrawSceneComponent();
		DrawRenderComponent();
		DrawCameraComponent();
		DrawLightComponent();

		ImGui::Spacing();
	}

	if (requestDestroyEntity != Entity::Null)
	{
		if (selectedEntity == requestDestroyEntity)
			selectedEntity = Entity::Null;

		DestroyEntity(requestDestroyEntity);
		requestDestroyEntity = Entity::Null;
	}
}

void EntityView::DrawEntityPropertyButtons()
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
		AddComponent(selectedEntity, addComponentType);

	ImGui::SameLine();

	if (ImGui::Button("Destroy entity"))
	{
		requestDestroyEntity = selectedEntity;
	}
}

void EntityView::DrawSceneComponent()
{
	SceneObjectId sceneObj = world->Lookup(selectedEntity);
	if (sceneObj != SceneObjectId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		if (ImGui::CollapsingHeader("Scene object", &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool edited = false;
			SceneEditTransform transform = world->GetEditTransform(sceneObj);

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
				world->SetEditTransform(sceneObj, transform);
		}

		if (componentVisible == false)
			world->RemoveSceneObject(sceneObj);
	}
}

void EntityView::DrawRenderComponent()
{
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

				std::strncpy(textInputBuffer, meshPath, TextInputBufferSize);
				if (ImGui::InputText("Mesh path", textInputBuffer, TextInputBufferSize))
				{
					MeshId newMeshId = meshManager->GetIdByPath(StringRef(textInputBuffer));

					if (newMeshId != MeshId::Null)
					{
						if (newMeshId != meshId)
						{
							renderer->SetMeshId(renderObj, newMeshId);
							
							SceneObjectId sceneObj = world->Lookup(selectedEntity);
							if (sceneObj != SceneObjectId::Null)
								world->MarkUpdated(sceneObj);
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

				std::strncpy(textInputBuffer, materialPath, TextInputBufferSize);
				if (ImGui::InputText("Material path", textInputBuffer, TextInputBufferSize))
				{
					MaterialId newMatId = materialManager->GetIdByPath(StringRef(textInputBuffer));

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

void EntityView::DrawCameraComponent()
{
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

void EntityView::DrawLightComponent()
{
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
				if (ImGui::DragFloat("Radius", &radius, 0.01f, 0.01f, 0.0f))
					lightManager->SetRadius(lightId, radius);
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

void EntityView::CreateEntity(ComponentType* components, unsigned int componentCount)
{
	Entity entity = entityManager->Create();

	for (unsigned int i = 0; i < componentCount; ++i)
		AddComponent(entity, components[i]);

	selectedEntity = entity;
	requestScrollToEntity = entity;
}

void EntityView::DestroyEntity(Entity entity)
{
	for (size_t i = 0; i < ComponentTypeCount; ++i)
		RemoveComponentIfExists(entity, static_cast<ComponentType>(i));

	entityManager->Destroy(entity);
}

void EntityView::AddComponent(Entity entity, ComponentType componentType)
{
	switch (componentType)
	{
	case EntityView::ComponentType::Scene:
	{
		SceneObjectId sceneObj = world->Lookup(entity);
		if (sceneObj == SceneObjectId::Null)
			world->AddSceneObject(entity);
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

void EntityView::RemoveComponentIfExists(Entity entity, ComponentType componentType)
{
	switch (componentType)
	{
	case EntityView::ComponentType::Scene:
	{
		SceneObjectId sceneObj = world->Lookup(entity);
		if (sceneObj != SceneObjectId::Null)
			world->RemoveSceneObject(sceneObj);
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
