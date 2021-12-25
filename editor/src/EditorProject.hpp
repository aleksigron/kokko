#pragma once

#include <filesystem>

#include "Core/String.hpp"

struct StringRef;

namespace kokko
{
namespace editor
{

class EditorProject
{
public:
	EditorProject(Allocator* allocator);

	// Uses rootPath to write a project.yml file
	bool SerializeToFile();

	// Tries to read project.yml file from the specified directory
	bool DeserializeFromFile(const std::filesystem::path& projectDirectory);

	const std::filesystem::path& GetRootPath() const;
	const String& GetRootPathString() const;
	void SetRootPath(const std::filesystem::path& path);

	const std::filesystem::path& GetAssetPath() const;

	const String& GetName() const;
	void SetName(StringRef name);

private:
	std::filesystem::path rootPath;
	String rootPathString;

	std::filesystem::path assetPath;

	String name;
};

}
}
