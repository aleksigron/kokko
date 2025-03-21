#include "SceneView.hpp"

#include "imgui.h"

#include "ImGuizmo.h"

#include "Core/Core.hpp"

#include "Engine/EntityManager.hpp"
#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/MeshComponentSystem.hpp"
#include "Rendering/RenderTypes.hpp"

#include "Resources/AssetLibrary.hpp"
#include "Resources/ModelManager.hpp"

#include "App/EditorContext.hpp"
#include "App/EditorConstants.hpp"

namespace kokko
{
namespace editor
{

SceneView::SceneView() :
	EditorWindow("Scene", EditorWindowGroup::Regular),
	contentWidth(0),
	contentHeight(0),
	currentGizmoOperation(ImGuizmo::TRANSLATE),
	currentGizmoMode(ImGuizmo::LOCAL),
	resizeRequested(false),
	windowIsFocused(false),
	windowIsHovered(false),
	modelManager(nullptr)
{
	Vec3f position(-3.0f, 2.0f, 6.0f);
	Vec3f target(0.0f, 1.0f, 0.0f);
	editorCamera.LookAt(position, target);
}

void SceneView::Initialize(render::Device* renderDevice, Window* window, const ResourceManagers& resourceManagers)
{
	framebuffer.SetRenderDevice(renderDevice);

	editorCamera.SetWindow(window);

	modelManager = resourceManagers.modelManager;
}

void SceneView::Update(EditorContext&)
{
	if (contentWidth > 0 && contentHeight > 0)
		editorCamera.SetAspectRatio(float(contentWidth), float(contentHeight));

	editorCamera.Update(windowIsFocused || windowIsHovered);
}

void SceneView::LateUpdate(EditorContext& context)
{
	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 regionMin = ImGui::GetWindowContentRegionMin();

			float contentRegionLeft = windowPos.x + regionMin.x;
			float contentRegionTop = windowPos.y + regionMin.y;

			if (ImGui::IsAnyMouseDown() == false &&
				size.x > 0 && size.y > 0 &&
				(size.x != contentWidth || size.y != contentHeight))
			{
				// Update framebuffer
				KK_LOG_INFO("Scene view content region resized to ({}, {})", size.x, size.y);

				contentWidth = static_cast<int>(size.x);
				contentHeight = static_cast<int>(size.y);

				resizeRequested = true;
			}

			ImVec2 startCursorPos = ImGui::GetCursorPos();

			if (framebuffer.IsInitialized())
			{
				ImVec2 uv0(0.0f, 1.0f);
				ImVec2 uv1(1.0f, 0.0f);

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(framebuffer.GetColorTextureId(0).i));

				ImGui::Image(texId, size, uv0, uv1);

				if (ImGui::BeginDragDropTarget())
				{
					const auto* payload = ImGui::AcceptDragDropPayload(EditorConstants::AssetDragDropType);
					if (payload != nullptr && payload->DataSize == sizeof(Uid))
					{
						Uid assetUid;
						std::memcpy(&assetUid, payload->Data, payload->DataSize);

						if (auto newAsset = context.assetLibrary->FindAssetByUid(assetUid))
						{
							if (newAsset->GetType() == AssetType::Model)
								HandleModelDragDrop(context, newAsset);
						}

					}

					ImGui::EndDragDropTarget();
				}
			}

			// Draw tool buttons

			// Start drawing on top of the image
			ImGui::SetCursorPos(startCursorPos);

			if (ImGui::Button("Translate"))
				currentGizmoOperation = ImGuizmo::TRANSLATE;

			ImGui::SameLine();
			if (ImGui::Button("Rotate"))
				currentGizmoOperation = ImGuizmo::ROTATE;

			ImGui::SameLine();
			if (ImGui::Button("Scale"))
				currentGizmoOperation = ImGuizmo::SCALE;

			ImGui::SameLine();
			if (ImGui::Button("Local"))
				currentGizmoMode = ImGuizmo::LOCAL;

			ImGui::SameLine();
			if (ImGui::Button("World"))
				currentGizmoMode = ImGuizmo::WORLD;

			// Draw gizmo

			ImGuizmo::OPERATION op = static_cast<ImGuizmo::OPERATION>(currentGizmoOperation);
			ImGuizmo::MODE mode = static_cast<ImGuizmo::MODE>(currentGizmoMode);

			Entity entity = context.selectedEntity;

			if (entity != Entity::Null)
			{
				World* world = context.world;
				Scene* scene = world->GetScene();
				SceneObjectId sceneObj = scene->Lookup(entity);

				if (sceneObj != SceneObjectId::Null)
				{
					float rectWidth = static_cast<float>(contentWidth);
					float rectHeight = static_cast<float>(contentHeight);

					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(contentRegionLeft, contentRegionTop, rectWidth, rectHeight);

					Mat4x4f viewTransform = editorCamera.GetCameraTransform().inverse;
					float* view = viewTransform.ValuePointer();

					Mat4x4f projectionTransform = editorCamera.GetProjectionParameters().GetProjectionMatrix(false);
					float* proj = projectionTransform.ValuePointer();

					Mat4x4f objectTransform = scene->GetWorldTransform(sceneObj);

					if (ImGuizmo::Manipulate(view, proj, op, mode, objectTransform.ValuePointer()))
					{
						Mat4x4f local;

						SceneObjectId parentObj = scene->GetParent(sceneObj);
						if (parentObj != SceneObjectId::Null)
						{
							Mat4x4f parent = scene->GetWorldTransform(parentObj);

							auto parentInvOpt = parent.GetInverse();
							if (parentInvOpt.HasValue())
								local = parentInvOpt.GetValue() * objectTransform;
						}
						else
						{
							local = objectTransform;
						}

						SceneEditTransform editTransform;
						float* translate = editTransform.translation.ValuePointer();
						float* rotate = editTransform.rotation.ValuePointer();
						float* scale = editTransform.scale.ValuePointer();
						ImGuizmo::DecomposeMatrixToComponents(local.ValuePointer(), translate, rotate, scale);

						// DecomposeMatrixToComponents returns degrees, so convert to radians
						editTransform.rotation = Math::DegreesToRadians(editTransform.rotation);

						scene->SetLocalAndEditTransform(sceneObj, local, editTransform);
					}
				}
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		windowIsFocused = ImGui::IsWindowFocused();
		windowIsHovered = ImGui::IsWindowHovered();

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void SceneView::ReleaseEngineResources()
{
	framebuffer.Destroy();
}

void SceneView::ResizeFramebufferIfRequested()
{
	if (resizeRequested)
	{
		ResizeFramebuffer();

		resizeRequested = false;
	}
}

const kokko::render::Framebuffer& SceneView::GetFramebuffer()
{
	return framebuffer;
}

Vec2i SceneView::GetContentAreaSize()
{
	return Vec2i(contentWidth, contentHeight);
}

CameraParameters SceneView::GetCameraParameters() const
{
	return editorCamera.GetCameraParameters();
}

void SceneView::ResizeFramebuffer()
{
	if (framebuffer.IsInitialized())
		framebuffer.Destroy();

	RenderTextureSizedFormat format = RenderTextureSizedFormat::SRGB8;

	framebuffer.Create(contentWidth, contentHeight, Optional<RenderTextureSizedFormat>(), ArrayView(&format, 1));
}

void SceneView::HandleModelDragDrop(EditorContext& context, const AssetInfo* asset)
{
	ModelId modelId = modelManager->FindModelByUid(asset->GetUid());
	if (modelId != ModelId::Null)
	{
		ModelInfo model{
			modelId,
			modelManager->GetModelNodes(modelId),
			modelManager->GetModelMeshes(modelId),
			modelManager->GetModelMeshParts(modelId)
		};
		
		if (model.nodes.GetCount() != 0)
		{
			// Create hierarchy of nodes stored in the model

			for (int16_t index = 0, count = static_cast<int16_t>(model.nodes.GetCount()); index < count; ++index)
			{
				if (model.nodes[index].parent < 0)
					LoadModelNode(context, model, index, SceneObjectId::Null);
			}
		}
		else
		{
			// Only create one node and choose the first mesh for it
			KK_LOG_INFO("No nodes");
		}
	}
}

void SceneView::LoadModelNode(
	EditorContext& context, const ModelInfo& model, int16_t nodeIndex, SceneObjectId parent)
{
	const ModelNode& node = model.nodes[nodeIndex];

	// TODO: Do actual loading
	EntityManager* entityManager = context.world->GetEntityManager();
	Scene* scene = context.world->GetScene();

	Entity entity = entityManager->Create();

	// Handle scene object creation

	SceneObjectId objId = scene->AddSceneObject(entity);

	SceneEditTransform editTransform;
	float* translate = editTransform.translation.ValuePointer();
	float* rotate = editTransform.rotation.ValuePointer();
	float* scale = editTransform.scale.ValuePointer();
	ImGuizmo::DecomposeMatrixToComponents(node.transform.ValuePointer(), translate, rotate, scale);

	// DecomposeMatrixToComponents returns degrees, so convert to radians
	editTransform.rotation = Math::DegreesToRadians(editTransform.rotation);

	scene->SetLocalAndEditTransform(objId, node.transform, editTransform);
	if (parent != SceneObjectId::Null)
		scene->SetParent(objId, parent);

	// Handle mesh component creation

	if (node.meshIndex >= 0)
	{
		MeshComponentSystem* meshCompSystem = context.world->GetMeshComponentSystem();
		MeshComponentId meshCompId = meshCompSystem->AddComponent(entity);
		MeshId meshId{ model.modelId, static_cast<uint32_t>(node.meshIndex) };
		meshCompSystem->SetMesh(meshCompId, meshId, model.meshes[node.meshIndex].partCount);
	}

	// Handle child nodes
	for (int16_t childIdx = node.firstChild; childIdx >= 0;)
	{
		LoadModelNode(context, model, childIdx, objId);

		childIdx = model.nodes[childIdx].nextSibling;
	}
}

} // namespace editor
} // namespace kokko
