#include "Editor/DebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Editor/EditorWindowInfo.hpp"

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
			const char* timingFormat = "Frametime: %.2f ms";
			char buffer[64];

			float currentFrameTime = Time::GetDeltaTime();
			std::snprintf(buffer, sizeof(buffer), timingFormat, currentFrameTime * 1000.0);
			ImGui::Text(buffer);

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
