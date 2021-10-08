#pragma once

#include <cstdint>
#include <filesystem>

namespace kokko
{
namespace editor
{

class FilePickerDialog
{
public:
	FilePickerDialog();

	static FilePickerDialog* Get();

	void Update();

	/*
	* Returns true when a dialog with the specied ID has finished.
	* If true, parameter pathOut is assigned the selected path, or an empty string,
	* if the dialog was cancelled.
	*/
	bool GetDialogResult(uint32_t id, std::filesystem::path& pathOut);

	uint32_t StartDialogFileOpen(const char* popupTitle, const char* actionText);
	uint32_t StartDialogFileSave(const char* popupTitle, const char* actionText);
	uint32_t StartDialogFolderOpen(const char* popupTitle, const char* actionText);

private:
	enum class DialogType
	{
		None,
		FileOpen,
		FileSave,
		FolderOpen,
	};

	uint32_t StartDialogInternal(const char* title, const char* action, DialogType type);

	static FilePickerDialog* singletonInstance;

	std::filesystem::path currentPath;
	std::filesystem::path selectedFilePath;

	bool dialogClosed;
	uint32_t closedTitleHash;
	std::filesystem::path resultPath;

	DialogType currentDialogType;
	uint32_t currentTitleHash;
	const char* currentTitle;
	const char* currentActionText;

	void CloseDialog(bool canceled);
};

}
}
