#pragma once

#include <filesystem>

#include "Core/String.hpp"

class Allocator;

struct EditorWindowInfo;

class AssetBrowserView
{
public:
	AssetBrowserView();
	~AssetBrowserView();

	void Draw(EditorWindowInfo& windowInfo);

private:
	void UpdateDirectoryListing();

	std::filesystem::path currentPath;
	std::filesystem::path selectedPath;
};
