#pragma once

#include <filesystem>

#include "Core/String.hpp"

class EditorProject
{
public:
	EditorProject(Allocator* allocator);

	// Uses rootPath to write a project.yml file
	bool SerializeToFile();

	// Tries to read project.yml file from the specified directory
	bool DeserializeFromFile(const std::filesystem::path& projectDirectory);

	std::string GetRootPath() const;
	const String& GetName() const;

private:
	void SetRootPath(const std::filesystem::path& path);

	std::filesystem::path rootPath;
	String rootPathString;
	String name;
};
