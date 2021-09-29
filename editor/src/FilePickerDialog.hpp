#pragma once

#include <filesystem>

class FilePickerDialog
{
public:
	enum class DialogType
	{
		Unknown,
		FileOpen,
		FileSave
	};

	FilePickerDialog();

	static FilePickerDialog* GetInstance();

	void StartDialogFileOpen(const char* popupTitle, const char* actionText);
	void StartDialogFileSave(const char* popupTitle, const char* actionText);

	/*
	* Update file picker dialog UI.
	* Returns true when the dialog has finished (when it has closed).
	* Parameter pathOut is assigned the selected path, or an empty string,
	* if the dialog was cancelled.
	*/
	bool Update(std::filesystem::path& pathOut);

	/*
	* Returns the dialog type of the current or last dialog that was open.
	*/
	DialogType GetLastDialogType();

private:
	static FilePickerDialog* singletonInstance;

	std::filesystem::path currentPath;
	std::filesystem::path selectedFilePath;

	DialogType dialogType;
	const char* currentTitle;
	const char* currentActionText;

	void CloseDialog();
};
