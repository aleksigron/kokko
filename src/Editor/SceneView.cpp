#include "Editor/SceneView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

SceneView::SceneView() :
	contentWidth(0),
	contentHeight(0)
{
}

void SceneView::Initialize(RenderDevice* renderDevice)
{
	framebuffer.SetRenderDevice(renderDevice);
}

void SceneView::Draw()
{
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	if (ImGui::Begin("Scene"))
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

			ResizeFramebuffer();
		}

		if (framebuffer.IsInitialized())
		{
			ImVec2 uv0(0.0f, 1.0f);
			ImVec2 uv1(1.0f, 0.0f);

			void* texId = reinterpret_cast<void*>(static_cast<size_t>(framebuffer.GetColorTextureId(0)));

			ImGui::Image(texId, size, uv0, uv1);
		}
	}

	ImGui::End();
	ImGui::PopStyleVar();
}

const Framebuffer& SceneView::GetFramebuffer()
{
	return framebuffer;
}

void SceneView::ResizeFramebuffer()
{
	if (framebuffer.IsInitialized())
		framebuffer.Destroy();

	RenderTextureSizedFormat format = RenderTextureSizedFormat::SRGB8;

	framebuffer.Create(contentWidth, contentHeight,
		Optional<RenderTextureSizedFormat>(), ArrayView<RenderTextureSizedFormat>(&format, 1));
}
