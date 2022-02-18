#pragma once

#include <filesystem>

#include "Core/Optional.hpp"
#include "Core/Uid.hpp"

namespace kokko
{
namespace editor
{

struct EditorUserSettings
{
	std::filesystem::path lastOpenedProject;
	Optional<Uid> lastOpenedLevel;

	bool windowMaximized = false;
	int windowWidth = 0;
	int windowHeight = 0;

	bool SerializeToFile(const char* filePath);
	bool DeserializeFromFile(const char* filePath);
};

}
}
