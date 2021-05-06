#include "Editor/FilePickerDialog.hpp"

#include <cstdio>

#include "imgui.h"

void FilePickerDialog::FileOpen(const char* popupTitle)
{
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::BeginPopupModal(popupTitle, nullptr, flags))
	{
		char* path = "res/example/path";
		ImGui::Text("Select where to save the level");
		ImGui::InputText("##FilePickerDialogPath", path, strlen(path), ImGuiInputTextFlags_ReadOnly);

		ImVec2 listSize(0.0f, -ImGui::GetFrameHeightWithSpacing());
		if (ImGui::BeginChild("FilePickerDialogList", listSize, true))
		{
			ImGui::Selectable("..");

			for (int row = 0; row < 30; row++)
			{
				char buf[32];
				sprintf(buf, "File %d", row);

				ImGui::Selectable(buf);
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("OK", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}
