#include "DebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Engine/EngineSettings.hpp"

#include "System/Time.hpp"

#include "EditorContext.hpp"

namespace kokko
{
namespace editor
{

DebugView::DebugView() :
	EditorWindow("Debug"),
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
		if (ImGui::Begin(windowTitle, &windowIsOpen))
		{
			EngineSettings* engineSettings = context.engineSettings;

			float currentFrameTime = Time::GetDeltaTime();
			ImGui::Text("Frametime: %.2f ms", currentFrameTime * 1000.0);

			ImGui::Checkbox("Vertical sync", &engineSettings->verticalSync);
			ImGui::Checkbox("Draw mesh bounds", &engineSettings->drawMeshBounds);

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
