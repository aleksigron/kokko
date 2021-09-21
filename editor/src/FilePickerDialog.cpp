#include "FilePickerDialog.hpp"

#include "imgui.h"

#include "Core/CString.hpp"
#include "Core/String.hpp"

FilePickerDialog::FilePickerDialog() :
	dialogType(DialogType::Unknown),
	currentTitle(nullptr),
	currentActionText(nullptr)
{
}

void FilePickerDialog::StartDialogFileOpen(const char* popupTitle, const char* actionText)
{
	currentPath = std::filesystem::current_path();
	selectedFilePath = std::filesystem::path();

	dialogType = DialogType::FileOpen;

	currentTitle = popupTitle;
	currentActionText = actionText;

	ImGui::OpenPopup(currentTitle);
}

void FilePickerDialog::StartDialogFileSave(const char* popupTitle, const char* actionText)
{
	currentPath = std::filesystem::current_path();
	selectedFilePath = std::filesystem::path();

	dialogType = DialogType::FileSave;

	currentTitle = popupTitle;
	currentActionText = actionText;

	ImGui::OpenPopup(currentTitle);
}

bool FilePickerDialog::Update(std::filesystem::path& pathOut)
{
	namespace fs = std::filesystem;

	if (currentTitle == nullptr)
		return false;

	bool dialogClosed = false;

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::BeginPopupModal(currentTitle, nullptr, flags))
	{
		bool existingFileIsSelected = false;

		ImGui::Text("Select where to save the level");

		std::string currentPathStr = currentPath.u8string();

		// Directory path text

		ImGuiInputTextFlags dirTextFlags = ImGuiInputTextFlags_ReadOnly;
		ImGui::SetNextItemWidth(-FLT_MIN);
		ImGui::InputText("##FilePickerDialogDir", currentPathStr.data(), currentPathStr.length(), dirTextFlags);

		// Directory entries

		ImVec2 listSize(0.0f, -2.0f * ImGui::GetFrameHeightWithSpacing());
		if (ImGui::BeginChild("FilePickerDialogList", listSize, true))
		{
			if (ImGui::Selectable(".."))
			{
				// Move up a directory
				currentPath = currentPath.parent_path();
				selectedFilePath = fs::path();
			}

			for (fs::directory_iterator itr(currentPath), end; itr != end; ++itr)
			{
				bool isDir = itr->is_directory();
				bool isFile = itr->is_regular_file();

				fs::path path = itr->path();
				std::string pathStr = path.filename().u8string();

				ImGuiSelectableFlags entryFlags = ImGuiSelectableFlags_AllowDoubleClick;
				if (isDir == false && isFile == false)
					entryFlags |= ImGuiSelectableFlags_Disabled;

				bool selected = false;
				if (path == selectedFilePath)
				{
					existingFileIsSelected = true;
					selected = true;
				}

				if (ImGui::Selectable(pathStr.c_str(), selected, entryFlags))
				{
					if (isDir)
					{
						// Move into directory
						currentPath = path;
						selectedFilePath = fs::path();
					}
					else if (isFile)
					{
						// Select file
						selectedFilePath = path;
					}
				}
			}
		}
		ImGui::EndChild();

		char inputBuf[512];

		// Selected file name input

		if (selectedFilePath.has_filename())
		{
			std::string fileName = selectedFilePath.filename().u8string();
			StringCopySafe(inputBuf, fileName.c_str());
		}
		else
			SetEmptyString(inputBuf);

		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::InputText("Name", inputBuf, sizeof(inputBuf)))
		{
			selectedFilePath = currentPath / fs::path(inputBuf);
		}

		// Buttons

		float fontSize = ImGui::GetFontSize();
		ImVec2 buttonSize(fontSize * 7.0f, 0.0f);

		if (ImGui::Button(currentActionText, buttonSize))
		{
			if (existingFileIsSelected || dialogType == DialogType::FileSave)
			{
				dialogClosed = true;

				pathOut = selectedFilePath;

				CloseDialog();
			}
			// TODO: Should make the button look disabled when there's no valid selection
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", buttonSize))
		{
			dialogClosed = true;

			CloseDialog();
		}

		ImGui::EndPopup();
	}

	return dialogClosed;
}

FilePickerDialog::DialogType FilePickerDialog::GetLastDialogType()
{
	return dialogType;
}

void FilePickerDialog::CloseDialog()
{
	currentTitle = nullptr;
	currentActionText = nullptr;
	ImGui::CloseCurrentPopup();
}
