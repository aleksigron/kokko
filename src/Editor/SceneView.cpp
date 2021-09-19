#include "Editor/SceneView.hpp"

#include "imgui.h"

#include "ImGuizmo.h"

#include "Core/Core.hpp"

#include "Editor/EditorWindowInfo.hpp"
#include "Editor/SelectionContext.hpp"

#include "Engine/World.hpp"

#include "Graphics/Scene.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDeviceEnums.hpp"

SceneView::SceneView() :
	contentWidth(0),
	contentHeight(0),
	currentGizmoOperation(ImGuizmo::TRANSLATE),
	resizeRequested(false),
	windowIsFocused(false),
	windowIsHovered(false)
{
	Vec3f position(-3.0f, 2.0f, 6.0f);
	Vec3f target(0.0f, 1.0f, 0.0f);
	editorCamera.LookAt(position, target);
}

void SceneView::Initialize(RenderDevice* renderDevice, InputManager* inputManager)
{
	framebuffer.SetRenderDevice(renderDevice);

	editorCamera.SetInputManager(inputManager);
}

void SceneView::Update()
{
	if (contentWidth > 0 && contentHeight > 0)
		editorCamera.SetAspectRatio(contentWidth, contentHeight);

	editorCamera.Update(windowIsFocused || windowIsHovered);
}

void SceneView::Draw(EditorWindowInfo& windowInfo, World* world, SelectionContext& selectionContext)
{
	if (windowInfo.isOpen)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin(windowInfo.title, &windowInfo.isOpen))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();
			ImVec2 windowPos = ImGui::GetWindowPos();
			ImVec2 regionMin = ImGui::GetWindowContentRegionMin();

			int contentRegionLeft = windowPos.x + regionMin.x;
			int contentRegionTop = windowPos.y + regionMin.y;

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

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(framebuffer.GetColorTextureId(0)));

				ImGui::Image(texId, size, uv0, uv1);
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

			// Draw gizmo

			ImGuizmo::OPERATION op = static_cast<ImGuizmo::OPERATION>(currentGizmoOperation);
			ImGuizmo::MODE mode = ImGuizmo::LOCAL;

			Entity entity = selectionContext.selectedEntity;

			if (entity != Entity::Null)
			{
				Scene* scene = world->GetScene();
				SceneObjectId sceneObj = scene->Lookup(entity);

				if (sceneObj != SceneObjectId::Null)
				{
					ImGuizmo::SetDrawlist();
					ImGuizmo::SetRect(contentRegionLeft, contentRegionTop, contentWidth, contentHeight);

					Mat4x4f viewTransform = editorCamera.GetCameraTransform().inverse;
					float* view = viewTransform.ValuePointer();

					Mat4x4f projectionTransform = editorCamera.GetProjectionParameters().GetProjectionMatrix(false);
					float* proj = projectionTransform.ValuePointer();

					Mat4x4f objectTransform = scene->GetLocalTransform(sceneObj);
					float* obj = objectTransform.ValuePointer();

					if (ImGuizmo::Manipulate(view, proj, op, mode, obj))
					{
						SceneEditTransform editTransform;
						float* translation = editTransform.translation.ValuePointer();
						float* rotation = editTransform.rotation.ValuePointer();
						float* scale = editTransform.scale.ValuePointer();

						ImGuizmo::DecomposeMatrixToComponents(obj, translation, rotation, scale);

						// DecomposeMatrixToComponents returns degrees, so convert to radians
						editTransform.rotation = Math::DegreesToRadians(editTransform.rotation);

						scene->SetLocalAndEditTransform(sceneObj, objectTransform, editTransform);
					}
				}
			}
		}

		if (windowInfo.requestFocus)
			ImGui::SetWindowFocus();

		windowIsFocused = ImGui::IsWindowFocused();
		windowIsHovered = ImGui::IsWindowHovered();

		ImGui::End();
		ImGui::PopStyleVar();
	}
}

void SceneView::ResizeFramebufferIfRequested()
{
	if (resizeRequested)
	{
		ResizeFramebuffer();

		resizeRequested = false;
	}
}

const Framebuffer& SceneView::GetFramebuffer()
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

	framebuffer.Create(contentWidth, contentHeight,
		Optional<RenderTextureSizedFormat>(), ArrayView<RenderTextureSizedFormat>(&format, 1));
}
