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
	static const char* const EditorResourcePath;

	// Virtual filesystem

	static const char* const VirtualMountEditor;

	// Editor settings

	static const char* const UserSettingsFilePath;

	// UI

	static const char* const AssetDirectoryName;
	static const char* const SceneDragDropType;
	static const char* const AssetDragDropType;

	// Model mesh needs its own type because its payload contains
	// the model UID as well as the mesh index
	static const char* const ModelMeshDragDropType;
};

}
}
