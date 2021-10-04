#include "FilePickerDialog.hpp"

#include <cstring>

#include "imgui.h"

#include "Core/CString.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

FilePickerDialog* FilePickerDialog::singletonInstance = nullptr;

FilePickerDialog::FilePickerDialog() :
	dialogClosed(false),
	closedTitleHash(0),
	currentDialogType(DialogType::None),
	currentTitleHash(0),
	currentTitle(nullptr),
	currentActionText(nullptr)
{
}

FilePickerDialog* FilePickerDialog::Get()
{
	if (singletonInstance == nullptr)
		singletonInstance = new FilePickerDialog();

	return singletonInstance;
}

void FilePickerDialog::Update()
{
	namespace fs = std::filesystem;

	dialogClosed = false;

	if (currentTitle == nullptr)
		return;

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	
	float fontSize = ImGui::GetFontSize();
	ImVec2 initialSize(fontSize * 20.0f, fontSize * 15.0f);
	ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);

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

		ImGuiInputTextFlags inputFlags;
		const char* inputTitle;

		if (currentDialogType == DialogType::FolderOpen)
		{
			inputFlags = ImGuiInputTextFlags_ReadOnly;
			inputTitle = "Folder";

			std::string fileName = currentPath.filename().u8string();
			StringCopySafe(inputBuf, fileName.c_str());
		}
		else
		{
			if (selectedFilePath.has_filename())
			{
				std::string fileName = selectedFilePath.filename().u8string();
				StringCopySafe(inputBuf, fileName.c_str());
			}
			else
				SetEmptyString(inputBuf);

			inputFlags = ImGuiInputTextFlags_None;
			inputTitle = "Filename";
		}

		ImGui::SetNextItemWidth(-FLT_MIN);
		if (ImGui::InputText(inputTitle, inputBuf, sizeof(inputBuf), inputFlags))
		{
			selectedFilePath = currentPath / fs::path(inputBuf);
		}

		// Buttons

		float fontSize = ImGui::GetFontSize();
		ImVec2 buttonSize(fontSize * 7.0f, 0.0f);

		if (ImGui::Button(currentActionText, buttonSize))
		{
			if (existingFileIsSelected ||
				currentDialogType == DialogType::FileSave ||
				currentDialogType == DialogType::FolderOpen)
			{
				CloseDialog(false);
			}
			// TODO: Should make the button look disabled when there's no valid selection
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", buttonSize))
		{
			CloseDialog(true);
		}

		ImGui::EndPopup();
	}
}

bool FilePickerDialog::GetDialogResult(uint32_t hash, std::filesystem::path& pathOut)
{
	if (dialogClosed && hash == closedTitleHash)
	{
		pathOut = std::move(resultPath);
		
		resultPath = std::filesystem::path();
		dialogClosed = false;
		closedTitleHash = 0;

		return true;
	}

	return false;
}

uint32_t FilePickerDialog::StartDialogFileOpen(const char* popupTitle, const char* actionText)
{
	return StartDialogInternal(popupTitle, actionText, DialogType::FileOpen);
}

uint32_t FilePickerDialog::StartDialogFileSave(const char* popupTitle, const char* actionText)
{
	return StartDialogInternal(popupTitle, actionText, DialogType::FileSave);
}

uint32_t FilePickerDialog::StartDialogFolderOpen(const char* popupTitle, const char* actionText)
{
	return StartDialogInternal(popupTitle, actionText, DialogType::FolderOpen);
}

uint32_t FilePickerDialog::StartDialogInternal(const char* title, const char* action, DialogType type)
{
	currentPath = std::filesystem::current_path();
	selectedFilePath = std::filesystem::path();

	currentDialogType = type;
	currentTitle = title;
	currentActionText = action;
	currentTitleHash = Hash::FNV1a_32(title, std::strlen(title));

	ImGui::OpenPopup(currentTitle);

	return currentTitleHash;
}

void FilePickerDialog::CloseDialog(bool canceled)
{
	// Results

	dialogClosed = true;
	closedTitleHash = currentTitleHash;

	if (canceled)
		resultPath = std::filesystem::path();
	else if (currentDialogType == DialogType::FolderOpen)
		resultPath = currentPath;
	else
		resultPath = selectedFilePath;

	// Clear current info
	currentDialogType = DialogType::None;
	currentTitle = nullptr;
	currentActionText = nullptr;
	currentTitleHash = 0;

	ImGui::CloseCurrentPopup();
}
