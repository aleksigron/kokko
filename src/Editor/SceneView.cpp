#include "Editor/SceneView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Editor/EditorWindowInfo.hpp"

#include "Rendering/CameraParameters.hpp"
#include "Rendering/RenderDeviceEnums.hpp"

SceneView::SceneView() :
	contentWidth(0),
	contentHeight(0),
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

void SceneView::Draw(EditorWindowInfo& windowInfo)
{
	if (windowInfo.isOpen)
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		if (ImGui::Begin(windowInfo.title, &windowInfo.isOpen))
		{
			ImVec2 size = ImGui::GetContentRegionAvail();

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

			if (framebuffer.IsInitialized())
			{
				ImVec2 uv0(0.0f, 1.0f);
				ImVec2 uv1(1.0f, 0.0f);

				void* texId = reinterpret_cast<void*>(static_cast<size_t>(framebuffer.GetColorTextureId(0)));

				ImGui::Image(texId, size, uv0, uv1);
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
