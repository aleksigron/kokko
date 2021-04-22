#include "Editor/EntityView.hpp"

#include "imgui.h"

#include "Entity/EntityManager.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MeshManager.hpp"

#include "Scene/Scene.hpp"

EntityView::EntityView() :
	entityManager(nullptr),
	cameraSystem(nullptr),
	lightManager(nullptr),
	renderer(nullptr),
	meshManager(nullptr),
	scene(nullptr),
	selectedEntity(Entity::Null)
{
}

void EntityView::Draw(EntityManager* entityManager, CameraSystem* cameraSystem,
	LightManager* lightManager, Renderer* renderer, MeshManager* meshManager, Scene* scene)
{
	this->entityManager = entityManager;
	this->cameraSystem = cameraSystem;
	this->lightManager = lightManager;
	this->renderer = renderer;
	this->meshManager = meshManager;
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

		DrawSceneComponent();
		DrawRenderComponent();
		DrawCameraComponent();
		DrawLightComponent();
	}
}

void EntityView::DrawSceneComponent()
{
	SceneObjectId sceneObj = scene->Lookup(selectedEntity);
	if (sceneObj != SceneObjectId::Null)
	{
		if (ImGui::TreeNodeEx("Scene object", ImGuiTreeNodeFlags_SpanAvailWidth))
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

void EntityView::DrawRenderComponent()
{
	RenderObjectId renderObj = renderer->Lookup(selectedEntity);
	if (renderObj != RenderObjectId::Null)
	{
		if (ImGui::TreeNodeEx("Render object", ImGuiTreeNodeFlags_SpanAvailWidth))
		{
			MeshId meshId = renderer->GetMeshId(renderObj);
			const char* meshPath = meshManager->GetPath(meshId);

			if (meshPath != nullptr)
			{
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
					{
						ImVec4 warningColor(1.0f, 0.6f, 0.0f, 1.0f);
						ImGui::TextColored(warningColor, "Mesh not found");
					}
				}
			}
			else // We currently only support editing meshes that have been loaded from file
			{
				ImGui::Text("Mesh %u", meshId.i);
			}

			RenderOrderData order = renderer->GetOrderData(renderObj);

			ImGui::Text("Material %u", order.material.i);

			ImGui::TreePop();
		}
	}
}

void EntityView::DrawCameraComponent()
{
	CameraId cameraId = cameraSystem->Lookup(selectedEntity);
	if (cameraId != CameraId::Null)
	{
		if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_SpanAvailWidth))
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

			ImGui::TreePop();
		}
	}
}

void EntityView::DrawLightComponent()
{
	LightId lightId = lightManager->Lookup(selectedEntity);
	if (lightId != LightId::Null)
	{
		if (ImGui::TreeNodeEx("Light", ImGuiTreeNodeFlags_SpanAvailWidth))
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
					lightManager->SetRadius(lightId, spotAngle);
				}
			}

			// Currently shadows are only supported on directional lights
			if (lightType == LightType::Directional)
			{
				bool shadowCasting = lightManager->GetShadowCasting(lightId);
				if (ImGui::Checkbox("Cast shadows", &shadowCasting))
					lightManager->SetShadowCasting(lightId, shadowCasting);
			}

			ImGui::TreePop();
		}
	}
}
