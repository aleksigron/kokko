#pragma once

#include <filesystem>

namespace kokko
{
namespace editor
{

class EditorConstants
{
public:
	static const std::filesystem::path MetadataExtension;

	static const char* const AssetDirectoryName;

	static const char* const SceneDragDropType;
	static const char* const AssetDragDropType;
};

}
}
