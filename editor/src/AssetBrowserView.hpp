#pragma once

#include <filesystem>

#include "Core/String.hpp"

class Allocator;
class EditorImages;

struct EditorWindowInfo;

class AssetBrowserView
{
public:
	AssetBrowserView();
	~AssetBrowserView();

	void Initialize(const EditorImages* editorImages);

	void Draw(EditorWindowInfo& windowInfo);

private:
	void SetUpColumns(int columnCount, float columnWidth);
	void DrawEntry(const std::filesystem::directory_iterator& entry, float columnWidth);
	void UpdateDirectoryListing();

	std::filesystem::path currentPath;
	std::filesystem::path selectedPath;

	const EditorImages* editorImages;
};
