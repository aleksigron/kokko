#include "DebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Engine/EngineSettings.hpp"

#include "System/Time.hpp"

#include "App/EditorContext.hpp"

namespace kokko
{
namespace editor
{

DebugView::DebugView() :
	EditorWindow("Debug", EditorWindowGroup::Debug),
	debug(nullptr)
{
}

void DebugView::Initialize(Debug* debug)
{
	this->debug = debug;
}

void DebugView::Update(EditorContext& context)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowIsOpen)
	{
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			EngineSettings* engineSettings = context.engineSettings;

			float currentFrameTime = Time::GetDeltaTime();
			ImGui::Text("Frametime: %.2f ms", currentFrameTime * 1000.0);

			ImGui::Checkbox("Vertical sync", &engineSettings->verticalSync);

			kokko::RenderDebugSettings& features = engineSettings->renderDebug;

			bool drawBounds = features.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawBounds);
			if (ImGui::Checkbox("Draw mesh bounds", &drawBounds))
				features.SetFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawBounds, drawBounds);

			bool drawNormals = features.IsFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawNormals);
			if (ImGui::Checkbox("Draw mesh normals", &drawNormals))
				features.SetFeatureEnabled(kokko::RenderDebugFeatureFlag::DrawNormals, drawNormals);

			if (ImGui::Button("Capture profile"))
			{
				debug->RequestBeginProfileSession();
			}
		}

		if (requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}

}
}
