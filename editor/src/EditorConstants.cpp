#include "EditorConstants.hpp"

namespace kokko
{
namespace editor
{
const std::filesystem::path EditorConstants::MetadataExtension = std::filesystem::path(".meta");
const char* const EditorConstants::AssetDirectoryName = "Assets";

const char* const EditorConstants::EngineResourcePath = "engine/res";
const char* const EditorConstants::EditorResourcePath = "editor/res";

const char* const EditorConstants::VirtualPathEngine = "engine";
const char* const EditorConstants::VirtualPathEditor = "editor";
const char* const EditorConstants::VirtualPathAssets = "assets";

const char* const EditorConstants::SceneDragDropType = "SceneObject";
const char* const EditorConstants::AssetDragDropType = "Asset";
const char* const EditorConstants::MaterialDragDropType = "Material";


}
}
