#include "DebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "EditorWindowInfo.hpp"

#include "Engine/EngineSettings.hpp"

#include "System/Time.hpp"

DebugView::DebugView() : debug(nullptr)
{
}

void DebugView::Initialize(Debug* debug)
{
	this->debug = debug;
}

void DebugView::Draw(EditorWindowInfo& windowInfo, EngineSettings* engineSettings)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowInfo.isOpen)
	{
		if (ImGui::Begin(windowInfo.title, &windowInfo.isOpen))
		{
			float currentFrameTime = Time::GetDeltaTime();
			ImGui::Text("Frametime: %.2f ms", currentFrameTime * 1000.0);

			ImGui::Checkbox("Vertical sync", &engineSettings->verticalSync);
			ImGui::Checkbox("Draw mesh bounds", &engineSettings->drawMeshBounds);

			if (ImGui::Button("Capture profile"))
			{
				debug->RequestBeginProfileSession();
			}
		}

		if (windowInfo.requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}
