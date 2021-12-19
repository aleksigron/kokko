#include "EntityView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"
#include "Core/CString.hpp"

#include "Engine/EntityFactory.hpp"
#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/ParticleSystem.hpp"
#include "Graphics/Scene.hpp"
#include "Graphics/TerrainSystem.hpp"

#include "Rendering/CameraSystem.hpp"
#include "Rendering/LightManager.hpp"
#include "Rendering/Renderer.hpp"

#include "Resources/MaterialManager.hpp"
#include "Resources/MeshManager.hpp"

#include "AssetLibrary.hpp"
#include "EditorConstants.hpp"
#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

EntityView::EntityView() :
	EditorWindow("Properties"),
	materialManager(nullptr),
	meshManager(nullptr),
	requestDestroyEntity(Entity::Null)
{
}

void EntityView::Initialize(MaterialManager* materialManager, MeshManager* meshManager)
{
	this->materialManager = materialManager;
	this->meshManager = meshManager;
}

void EntityView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			World* world = context.world;
			EntityManager* entityManager = world->GetEntityManager();

			if (context.selectedEntity != Entity::Null)
			{
				// Name
				{
					char inputBuf[256];

					const char* entityName = entityManager->GetDebugNameWithFallback(context.selectedEntity);
					StringCopySafe(inputBuf, entityName);
					if (ImGui::InputText("Name", inputBuf, sizeof(inputBuf)))
					{
						if (StringIsEmpty(inputBuf) == false)
							entityManager->SetDebugName(context.selectedEntity, inputBuf);
						else
							entityManager->ClearDebugName(context.selectedEntity);
					}
				}

				DrawButtons(context.selectedEntity, world);

				DrawSceneComponent(context.selectedEntity, world);
				DrawRenderComponent(context);
				DrawCameraComponent(context.selectedEntity, world);
				DrawLightComponent(context.selectedEntity, world);
				DrawTerrainComponent(context.selectedEntity, world);
				DrawParticleComponent(context.selectedEntity, world);

				ImGui::Spacing();
			}

			if (requestDestroyEntity != Entity::Null)
			{
				if (context.selectedEntity == requestDestroyEntity)
					context.selectedEntity = Entity::Null;

				EntityFactory::DestroyEntity(world, requestDestroyEntity);
				requestDestroyEntity = Entity::Null;
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

void EntityView::DrawButtons(Entity selectedEntity, World* world)
{
	bool addComponent = false;
	EntityComponentType addComponentType;

	ImGui::PushItemWidth(ImGui::GetFontSize() * 9.0f);
	if (ImGui::BeginCombo("##AddComponentCombo", "Add component...", ImGuiComboFlags_NoArrowButton))
	{
		for (size_t i = 0; i < EntityFactory::ComponentTypeCount; ++i)
		{
			if (ImGui::Selectable(EntityFactory::GetComponentTypeName(i), false))
			{
				addComponent = true;
				addComponentType = static_cast<EntityComponentType>(i);
			}
		}

		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	if (addComponent)
		EntityFactory::AddComponent(world, selectedEntity, addComponentType);

	ImGui::SameLine();

	if (ImGui::Button("Destroy entity"))
	{
		requestDestroyEntity = selectedEntity;
	}
}

void EntityView::DrawSceneComponent(Entity selectedEntity, World* world)
{
	Scene* scene = world->GetScene();
	SceneObjectId sceneObj = scene->Lookup(selectedEntity);

	if (sceneObj != SceneObjectId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Scene);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
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

void EntityView::DrawRenderComponent(EditorContext& context)
{
	Scene* scene = context.world->GetScene();
	Renderer* renderer = context.world->GetRenderer();

	RenderObjectId renderObj = renderer->Lookup(context.selectedEntity);
	if (renderObj != RenderObjectId::Null)
	{
		ImGui::Spacing();

		ImVec4 warningColor(1.0f, 0.6f, 0.0f, 1.0f);

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Render);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			char inputBuf[256];

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

				ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
				StringCopySafe(inputBuf, meshPath);
				ImGui::InputText("Mesh path", inputBuf, sizeof(inputBuf), flags);

				if (ImGui::BeginDragDropTarget())
				{
					const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
					if (payload != nullptr)
					{
						StringRef assetPath(static_cast<const char*>(payload->Data), payload->DataSize);

						MeshId newMeshId = meshManager->GetIdByPath(assetPath);
						if (newMeshId != MeshId::Null && newMeshId != meshId)
						{
							renderer->SetMeshId(renderObj, newMeshId);

							SceneObjectId sceneObj = scene->Lookup(context.selectedEntity);
							if (sceneObj != SceneObjectId::Null)
								scene->MarkUpdated(sceneObj);
						}
					}

					ImGui::EndDragDropTarget();
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
				Uid materialUid = materialManager->GetMaterialUid(materialId);

				if (auto asset = context.assetLibrary->FindAssetByUid(materialUid))
					materialPath = asset->filePath.GetCStr();
			}

			// Allow editing if material is loaded from a file, or if there is no material set
			if (materialPath != nullptr || materialId == MaterialId::Null)
			{
				if (materialPath == nullptr)
					materialPath = "";

				ImGuiInputTextFlags flags = ImGuiInputTextFlags_ReadOnly;
				StringCopySafe(inputBuf, materialPath);
				ImGui::InputText("Material", inputBuf, sizeof(inputBuf), flags);

				if (ImGui::BeginDragDropTarget())
				{
					const auto* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
					if (payload != nullptr)
					{
						Uid assetUid;
						std::memcpy(&assetUid, payload->Data, payload->DataSize);

						MaterialId newMatId = materialManager->FindMaterialByUid(assetUid);
						if (newMatId != MaterialId::Null && newMatId != materialId)
						{
							RenderOrderData order;
							order.material = newMatId;
							order.transparency = materialManager->GetMaterialTransparency(newMatId);

							renderer->SetOrderData(renderObj, order);
						}
					}
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

void EntityView::DrawCameraComponent(Entity selectedEntity, World* world)
{
	CameraSystem* cameraSystem = world->GetCameraSystem();
	CameraId cameraId = cameraSystem->Lookup(selectedEntity);

	if (cameraId != CameraId::Null())
	{
		ImGui::Spacing();

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Camera);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
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

void EntityView::DrawLightComponent(Entity selectedEntity, World* world)
{
	LightManager* lightManager = world->GetLightManager();
	LightId lightId = lightManager->Lookup(selectedEntity);

	if (lightId != LightId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Light);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
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

void EntityView::DrawTerrainComponent(Entity selectedEntity, World* world)
{
	TerrainSystem* terrainSystem = world->GetTerrainSystem();
	TerrainId terrainId = terrainSystem->Lookup(selectedEntity);

	if (terrainId != TerrainId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Terrain);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			bool edited = false;

			float terrainSize = terrainSystem->GetSize(terrainId);
			float heightOrigin = terrainSystem->GetBottom(terrainId);
			float heightRange = terrainSystem->GetHeight(terrainId);

			if (ImGui::DragFloat("Terrain size", &terrainSize, 1.0f, 1.0f))
				terrainSystem->SetSize(terrainId, terrainSize);

			if (ImGui::DragFloat("Height origin", &heightOrigin, 0.1f))
				terrainSystem->SetBottom(terrainId, heightOrigin);

			if (ImGui::DragFloat("Height range", &heightRange, 0.1f, 0.1f))
				terrainSystem->SetHeight(terrainId, heightRange);

			Vec2f textureScale = terrainSystem->GetTextureScale(terrainId);
			if (ImGui::DragFloat2("Texture scale", textureScale.ValuePointer(), 0.01f, 0.01f))
				terrainSystem->SetTextureScale(terrainId, textureScale);

			if (componentVisible == false)
				terrainSystem->RemoveTerrain(terrainId);
		}
	}
}

void EntityView::DrawParticleComponent(Entity selectedEntity, World* world)
{
	ParticleSystem* particleSystem = world->GetParticleSystem();
	ParticleEmitterId emitterId = particleSystem->Lookup(selectedEntity);

	if (emitterId != ParticleEmitterId::Null)
	{
		ImGui::Spacing();

		bool componentVisible = true;
		const char* componentTitle = EntityFactory::GetComponentTypeName(EntityComponentType::Particle);
		if (ImGui::CollapsingHeader(componentTitle, &componentVisible, ImGuiTreeNodeFlags_DefaultOpen))
		{
			float emitRate = particleSystem->GetEmitRate(emitterId);
			if (ImGui::DragFloat("Emit rate", &emitRate, 1.0f, 1.0f))
				particleSystem->SetEmitRate(emitterId, emitRate);

			if (componentVisible == false)
				particleSystem->RemoveEmitter(emitterId);
		}
	}
}

}
}
