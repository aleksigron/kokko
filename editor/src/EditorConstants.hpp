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

	static const char* const MetadataExtensionStr;
	static const std::filesystem::path MetadataExtension;

	static const char* const AssetDirectoryName;

	static const char* const EngineResourcePath;
	static const char* const EditorResourcePath;

	// Virtual filesystem

	static const char* const VirtualMountEngine;
	static const char* const VirtualMountEditor;
	static const char* const VirtualMountAssets;

	// Editor settings

	static const char* const UserSettingsFilePath;

	// UI

	static const char* const SceneDragDropType;
	static const char* const AssetDragDropType;
	static const char* const MaterialDragDropType;
};

}
}
