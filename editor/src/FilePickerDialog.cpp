#include "FilePickerDialog.hpp"

#include <cstring>

#include "imgui.h"

#include "Core/Core.hpp"
#include "Core/CString.hpp"
#include "Core/Hash.hpp"
#include "Core/String.hpp"

#include "EditorConstants.hpp"

namespace kokko
{
namespace editor
{

FilePickerDialog::FilePickerDialog() :
	dialogClosed(false),
	closedTitleHash(0),
	currentTitleHash(0),
	descriptor{}
{
}

void FilePickerDialog::Update()
{
	if (descriptor.popupTitle == nullptr)
		return;

	namespace fs = std::filesystem;

	dialogClosed = false;

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	float fontSize = ImGui::GetFontSize();
	ImVec2 initialSize(fontSize * 20.0f, fontSize * 15.0f);
	ImGui::SetNextWindowSize(initialSize, ImGuiCond_FirstUseEver);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
	if (ImGui::BeginPopupModal(descriptor.popupTitle, nullptr, flags))
	{
		bool existingFileIsSelected = false;

		ImGui::Text("Select where to save the level");

		std::string currentPathStr;
		if (descriptor.relativeToAssetPath)
			currentPathStr = (EditorConstants::AssetDirectoryName / currentPath).u8string();

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
				if (descriptor.relativeToAssetPath == false ||
					currentPath.empty() == false)
				{
					currentPath = currentPath.parent_path();
					selectedFilePath = fs::path();
				}
			}

			fs::path currentDirAbsolute;
			
			if (descriptor.relativeToAssetPath)
				currentDirAbsolute = descriptor.assetPath / currentPath;
			else
				currentDirAbsolute = currentPath;

			for (fs::directory_iterator itr(currentDirAbsolute), end; itr != end; ++itr)
			{
				bool isDir = itr->is_directory();
				bool isFile = itr->is_regular_file();

				const fs::path& path = itr->path();

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
						currentPath = ConvertPath(path);
						selectedFilePath = fs::path();
					}
					else if (isFile)
					{
						// Select file
						selectedFilePath = ConvertPath(path);
					}
				}
			}
		}
		ImGui::EndChild();

		char inputBuf[512];

		ImGuiInputTextFlags inputFlags;
		const char* inputTitle;

		if (descriptor.dialogType == Type::FolderOpen)
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

		if (ImGui::Button(descriptor.actionButtonText, buttonSize))
		{
			if (existingFileIsSelected ||
				descriptor.dialogType == Type::FileSave ||
				descriptor.dialogType == Type::FolderOpen)
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

bool FilePickerDialog::GetDialogResult(uint64_t hash, std::filesystem::path& pathOut)
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

uint64_t FilePickerDialog::StartDialog(const Descriptor& descriptor)
{
	if (descriptor.relativeToAssetPath)
		currentPath = std::filesystem::path();
	else
		currentPath = std::filesystem::current_path();

	selectedFilePath = std::filesystem::path();

	this->descriptor = descriptor;
	currentTitleHash = kokko::Hash64(descriptor.popupTitle, std::strlen(descriptor.popupTitle), 0);

	ImGui::OpenPopup(descriptor.popupTitle);

	return currentTitleHash;
}

void FilePickerDialog::CloseDialog(bool canceled)
{
	// Results

	dialogClosed = true;
	closedTitleHash = currentTitleHash;

	if (canceled)
		resultPath = std::filesystem::path();
	if (descriptor.dialogType == Type::FolderOpen)
		resultPath = currentPath;
	else
		resultPath = selectedFilePath;

	// Clear current info
	currentPath = std::filesystem::path();
	selectedFilePath = std::filesystem::path();
	currentTitleHash = 0;
	descriptor = Descriptor{};

	ImGui::CloseCurrentPopup();
}

std::filesystem::path FilePickerDialog::ConvertPath(const std::filesystem::path& path)
{
	if (descriptor.relativeToAssetPath)
	{
		std::error_code err;
		auto relative = std::filesystem::relative(path, descriptor.assetPath, err);

		if (err)
		{
			KK_LOG_ERROR("FilePickerDialog: path could not be made relative, error: {}", err.message().c_str());
			return std::filesystem::path();
		}
		else
			return relative;
	}
	else
		return path;
}

}
}
