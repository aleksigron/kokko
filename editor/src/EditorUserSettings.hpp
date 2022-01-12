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

	bool SerializeToFile(const char* filePath);
	bool DeserializeFromFile(const char* filePath);
};

}
}
