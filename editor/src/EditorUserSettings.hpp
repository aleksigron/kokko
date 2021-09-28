#pragma once

#include <filesystem>

struct EditorUserSettings
{
	std::filesystem::path lastOpenedProject;

	bool SerializeToFile(const char* filePath);
	bool DeserializeFromFile(const char* filePath);
};
