#include "Editor/DebugView.hpp"

#include "imgui.h"

#include "Core/Core.hpp"

#include "Debug/Debug.hpp"

#include "Editor/EditorWindowInfo.hpp"

#include "System/Time.hpp"

DebugView::DebugView() : debug(nullptr)
{
}

void DebugView::Initialize(Debug* debug)
{
	this->debug = debug;
}

void DebugView::Draw(EditorWindowInfo& windowInfo)
{
	KOKKO_PROFILE_FUNCTION();

	if (windowInfo.isOpen)
	{
		if (ImGui::Begin(windowInfo.title, &windowInfo.isOpen))
		{
			const char* timingFormat = "Frametime: %.3f ms";
			char buffer[64];

			float currentFrameTime = Time::GetDeltaTime();
			std::snprintf(buffer, sizeof(buffer), timingFormat, currentFrameTime * 1000.0);
			ImGui::Text(buffer);

			if (ImGui::Button("Capture profile"))
			{
				debug->RequestBeginProfileSession();
			}

			bool vsync = debug->GetVerticalSyncEnabled();
			if (ImGui::Checkbox("Vertical sync", &vsync))
				debug->SetVerticalSyncEnabled(vsync);
		}

		if (windowInfo.requestFocus)
			ImGui::SetWindowFocus();

		ImGui::End();
	}
}
