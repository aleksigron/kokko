#pragma once

#include <filesystem>

namespace kokko
{
namespace editor
{

struct EditorUserSettings
{
	std::filesystem::path lastOpenedProject;

	bool SerializeToFile(const char* filePath);
	bool DeserializeFromFile(const char* filePath);
};

}
}
