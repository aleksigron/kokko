#pragma once

#include <filesystem>

namespace kokko
{
namespace editor
{

class EditorConstants
{
public:
	// Asset Library

	static const std::filesystem::path MetadataExtension;

	static const char* const AssetDirectoryName;

	static const char* const EngineResourcePath;
	static const char* const EditorResourcePath;

	// Virtual filesystem

	static const char* const VirtualPathEngine;
	static const char* const VirtualPathEditor;
	static const char* const VirtualPathAssets;

	// UI

	static const char* const SceneDragDropType;
	static const char* const AssetDragDropType;
	static const char* const MaterialDragDropType;
};

}
}
