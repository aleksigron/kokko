#pragma once

#include <filesystem>

#include "Core/String.hpp"
#include "Core/StringView.hpp"

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
	void SetName(ConstStringView name);

private:
	Allocator* allocator;
	std::filesystem::path rootPath;
	String rootPathString;

	std::filesystem::path assetPath;

	String name;
};

} // namespace editor
} // namespace kokko
