#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Engine/Engine.hpp"

#include "Entity/EntityManager.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"

#include "Scene/Scene.hpp"

const char* const EntityView::ComponentNames[] = {
	"Scene object",
	"Render object",
	"Camera",
	"Light"
};

EntityView::EntityView() :
	entityManager(nullptr),
	renderer(nullptr),
	lightManager(nullptr),
	cameraSystem(nullptr),
	materialManager(nullptr),
	meshManager(nullptr),
	selectedEntity(Entity::Null),
	requestScrollToEntity(Entity::Null)
{
	std::memset(textInputBuffer, 0, TextInputBufferSize);
}

void EntityView::Initialize(Engine* engine)
{
	entityManager = engine->GetEntityManager();
	renderer = engine->GetRenderer();
	lightManager = engine->GetLightManager();
	cameraSystem = engine->GetCameraSystem();
	materialManager = engine->GetMaterialManager();
	meshManager = engine->GetMeshManager();
}

void EntityView::Draw(Scene* scene)
{
	ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoScrollbar);

	float fontSize = ImGui::GetFontSize();

	ImGuiTableFlags tableFlags =
		ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame |
		ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_NoClip;

	if (ImGui::BeginTable("EntityLayout", 2, tableFlags))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		ImGui::PushItemWidth(ImGui::GetFontSize() * 10.0f);
		if (ImGui::BeginCombo("##CreateEntityCombo", "Create new", ImGuiComboFlags_NoArrowButton))
		{
			if (ImGui::Selectable("Empty"))
			{
				CreateEntity(scene, nullptr, 0);
			}
			if (ImGui::Selectable("Scene object"))
			{
				ComponentType component = ComponentType::Scene;
				CreateEntity(scene, &component, 1);
			}
			if (ImGui::Selectable("Render object"))
			{
				ComponentType components[] = { ComponentType::Scene, ComponentType::Render };
				CreateEntity(scene, components, sizeof(components) / sizeof(components[0]));
			}
			if (ImGui::Selectable("Camera"))
			{
				ComponentType components[] = { ComponentType::Scene, ComponentType::Camera };
				CreateEntity(scene, components, sizeof(components) / sizeof(components[0]));
			}
			if (ImGui::Selectable("Light"))
			{
				ComponentType components[] = { ComponentType::Scene, ComponentType::Light };
				CreateEntity(scene, components, sizeof(components) / sizeof(components[0]));
			}

			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		if (ImGui::BeginChild("EntityList"))
		{
			for (Entity entity : *entityManager)
			{
				SceneObjectId sceneObj = scene->Lookup(entity);

				// Only draw root level objects, or entities that don't exist in the scene hierarchy
				if (sceneObj == SceneObjectId::Null || scene->GetParent(sceneObj) == SceneObjectId::Null)
					DrawEntityNode(scene, entity, sceneObj);
			}

			ImGui::EndChild();
		}

		ImGui::TableNextColumn();

		{
			ImGui::BeginChild("EntityProps", ImVec2(0.0f, 0.0f));
			DrawEntityProperties(scene);
			ImGui::EndChild();
		}

		ImGui::EndTable();
	}

	ImGui::End();
}

void EntityView::DrawEntityNode(Scene* scene, Entity entity, SceneObjectId sceneObj)
{
	SceneObjectId firstChild = SceneObjectId::Null;
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
	
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

			DrawEntityNode(scene, childEntity, child);

			child = scene->GetNextSibling(child);
		}

		ImGui::TreePop();
	}
}

void EntityView::DrawEntityProperties(Scene* scene)
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

		bool addComponent = false;
		ComponentType addComponentType;

		ImGui::PushItemWidth(ImGui::GetFontSize() * 9.0f);
		if (ImGui::BeginCombo("##AddComponentCombo", "Add component", ImGuiComboFlags_NoArrowButton))
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

		DrawSceneComponent(scene);
		DrawRenderComponent(scene);
		DrawCameraComponent();
		DrawLightComponent();

		if (addComponent)
			AddComponent(scene, selectedEntity, addComponentType);
	}
}

void EntityView::DrawSceneComponent(Scene* scene)
{
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

void EntityView::DrawRenderComponent(Scene* scene)
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
			static const char* projectionNames[] = { "Perspective", "Orthographic" };

			bool edited = false;
			ProjectionParameters params = cameraSystem->GetProjectionParameters(cameraId);

			if (ImGui::BeginCombo("Projection", projectionNames[static_cast<int>(params.projection)]))
			{
				for (int i = 0; i < 2; ++i)
				{
					bool isSelected = params.projection == static_cast<ProjectionType>(i);
					if (ImGui::Selectable(projectionNames[i], &isSelected))
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
				cameraSystem->SetProjectionParameters(cameraId, params);

			if (componentVisible == false)
				cameraSystem->RemoveCameraComponent(cameraId);
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

			static const char* typeNames[] = { "Directional", "Point", "Spot" };
			if (ImGui::BeginCombo("Type", typeNames[static_cast<int>(lightType)]))
			{
				for (int i = 0; i < 3; ++i)
				{
					bool isSelected = lightType == static_cast<LightType>(i);
					if (ImGui::Selectable(typeNames[i], &isSelected))
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

void EntityView::CreateEntity(Scene* scene, ComponentType* components, unsigned int componentCount)
{
	Entity entity = entityManager->Create();

	for (unsigned int i = 0; i < componentCount; ++i)
		AddComponent(scene, entity, components[i]);

	selectedEntity = entity;
	requestScrollToEntity = entity;
}

void EntityView::AddComponent(Scene* scene, Entity entity, ComponentType componentType)
{
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
			cameraSystem->AddCameraComponent(entity);
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
