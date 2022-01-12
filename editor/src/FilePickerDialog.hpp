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
	enum class Type
	{
		FileOpen,
		FileSave,
		FolderOpen,
	};

	struct Descriptor
	{
		const char* popupTitle;
		const char* descriptionText;
		const char* actionButtonText;

		Type dialogType;

		bool relativeToAssetPath;
		std::filesystem::path assetPath;
	};

	FilePickerDialog();

	void Update();

	/*
	* Returns true when a dialog with the specied ID has finished.
	* If true, parameter pathOut is assigned the selected path, or an empty string,
	* if the dialog was cancelled.
	*/
	bool GetDialogResult(uint64_t id, std::filesystem::path& pathOut);

	uint64_t StartDialog(const Descriptor& descriptor);

private:
	void CloseDialog(bool canceled);

	std::filesystem::path ConvertPath(const std::filesystem::path& path);

	std::filesystem::path currentPath;
	std::filesystem::path selectedFilePath;

	bool dialogClosed;
	uint64_t closedTitleHash;
	std::filesystem::path resultPath;

	uint64_t currentTitleHash;

	Descriptor descriptor;
};

}
}
